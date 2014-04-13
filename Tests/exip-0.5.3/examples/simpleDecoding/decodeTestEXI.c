/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file decodeTestEXI.c
 * @brief Testing the EXI decoder
 *
 * @date Oct 13, 2010
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id: decodeTestEXI.c 328 2013-10-30 16:00:10Z kjussakov $
 */
#include "EXIParser.h"
#include "stringManipulate.h"
#include <stdio.h>
#include <string.h>
#include "decodeTestEXI.h"

#define INPUT_BUFFER_SIZE 200
#define MAX_PREFIXES 10

struct appData
{
	unsigned char outputFormat;
	unsigned char expectAttributeData;
	char nameBuf[200];             // needed for the OUT_XML Output Format
	struct element* stack;         // needed for the OUT_XML Output Format
	unsigned char unclosedElement; 	 // needed for the OUT_XML Output Format
	char prefixes[MAX_PREFIXES][200]; // needed for the OUT_XML Output Format
	unsigned char prefixesCount; 	 // needed for the OUT_XML Output Format
};


// Stuff needed for the OUT_XML Output Format
// ******************************************
struct element {
	struct element* next;
	char* name;
};

static void push(struct element** stack, struct element* el);
static struct element* pop(struct element** stack);
static struct element* createElement(char* name);
static void destroyElement(struct element* el);

// returns != 0 if error
static char lookupPrefix(struct appData* aData, String ns, unsigned char* prxHit, unsigned char* prefixIndex);

// ******************************************

// Content Handler API
static errorCode sample_fatalError(const errorCode code, const char* msg, void* app_data);
static errorCode sample_startDocument(void* app_data);
static errorCode sample_endDocument(void* app_data);
static errorCode sample_startElement(QName qname, void* app_data);
static errorCode sample_endElement(void* app_data);
static errorCode sample_attribute(QName qname, void* app_data);
static errorCode sample_stringData(const String value, void* app_data);
static errorCode sample_decimalData(Decimal value, void* app_data);
static errorCode sample_intData(Integer int_val, void* app_data);
static errorCode sample_floatData(Float fl_val, void* app_data);
static errorCode sample_booleanData(boolean bool_val, void* app_data);
static errorCode sample_dateTimeData(EXIPDateTime dt_val, void* app_data);
static errorCode sample_binaryData(const char* binary_val, Index nbytes, void* app_data);
static errorCode sample_qnameData(const QName qname, void* app_data);

FILE *outfile = NULL; // Default output file

errorCode decode(EXIPSchema* schemaPtr, unsigned char outFlag, FILE *infile, boolean outOfBandOpts, EXIOptions* opts, size_t (*inputStream)(void* buf, size_t size, void* stream))
{
	Parser testParser;
	char buf[INPUT_BUFFER_SIZE];
	BinaryBuffer buffer;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	struct appData parsingData;

	buffer.buf = buf;
	buffer.bufLen = INPUT_BUFFER_SIZE;
	buffer.bufContent = 0;

	outfile = fopen("outputEXIP.xml", "w" );
	if(!outfile)
	{
		fprintf(stderr, "Unable to open file %s", "evalTestDecode.xml");
		return 1;
	}

	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any, otherwise set to NULL
	buffer.ioStrm.readWriteToStream = inputStream;
	buffer.ioStrm.stream = infile;

	// II: Second, initialize the parser object
	TRY(initParser(&testParser, buffer, &parsingData));

	// III: Initialize the parsing data and hook the callback handlers to the parser object.
	//      If out-of-band options are defined use testParser.strm.header.opts to set them
	parsingData.expectAttributeData = 0;
	parsingData.stack = NULL;
	parsingData.unclosedElement = 0;
	parsingData.prefixesCount = 0;
	parsingData.outputFormat = outFlag;
	if(outOfBandOpts && opts != NULL)
		testParser.strm.header.opts = *opts;

	testParser.handler.fatalError = sample_fatalError;
	testParser.handler.error = sample_fatalError;
	testParser.handler.startDocument = sample_startDocument;
	testParser.handler.endDocument = sample_endDocument;
	testParser.handler.startElement = sample_startElement;
	testParser.handler.attribute = sample_attribute;
	testParser.handler.stringData = sample_stringData;
	testParser.handler.endElement = sample_endElement;
	testParser.handler.decimalData = sample_decimalData;
	testParser.handler.intData = sample_intData;
	testParser.handler.floatData = sample_floatData;
	testParser.handler.booleanData = sample_booleanData;
	testParser.handler.dateTimeData = sample_dateTimeData;
	testParser.handler.binaryData = sample_binaryData;
	testParser.handler.qnameData = sample_qnameData;

	// IV: Parse the header of the stream

	TRY(parseHeader(&testParser, outOfBandOpts));

	// IV.1: Set the schema to be used for parsing.
	// The schemaID mode and schemaID field can be read at
	// parser.strm.header.opts.schemaIDMode and
	// parser.strm.header.opts.schemaID respectively
	// If schemaless mode, use setSchema(&parser, NULL);

	TRY(setSchema(&testParser, schemaPtr));

	// V: Parse the body of the EXI stream

	while(tmp_err_code == EXIP_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	// VI: Free the memory allocated by the parser

	destroyParser(&testParser);

	if(tmp_err_code == EXIP_PARSING_COMPLETE)
		return EXIP_OK;
	else
		return tmp_err_code;

	fclose(outfile);
}

static errorCode sample_fatalError(const errorCode code, const char* msg, void* app_data)
{
	fprintf(outfile, "\n%d : FATAL ERROR: %s\n", code, msg);
	return EXIP_HANDLER_STOP;
}

static errorCode sample_startDocument(void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
		fprintf(outfile, "SD\n");
	else if(appD->outputFormat == OUT_XML)
		fprintf(outfile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	return EXIP_OK;
}

static errorCode sample_endDocument(void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
		fprintf(outfile, "ED\n");
	else if(appD->outputFormat == OUT_XML)
		fprintf(outfile, "\n");

	return EXIP_OK;
}

static errorCode sample_startElement(QName qname, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
	{
		fprintf(outfile, "SE ");
		fprintf(outfile, "%.*s", qname.uri->length, qname.uri->str);
		fprintf(outfile, " ");
		fprintf(outfile, "%.*s", qname.localName->length, qname.localName->str);
		fprintf(outfile, "\n");
	}
	else if(appD->outputFormat == OUT_XML)
	{
		char error = 0;
		unsigned char prefixIndex = 0;
		unsigned char prxHit = 1;
		int t;

		if(!isStringEmpty(qname.uri))
		{
			error = lookupPrefix(appD, *qname.uri, &prxHit, &prefixIndex);
			if(error != 0)
				return EXIP_HANDLER_STOP;

			sprintf(appD->nameBuf, "p%d:", prefixIndex);
			t = strlen(appD->nameBuf);
			memcpy(appD->nameBuf + t, qname.localName->str, qname.localName->length);
			appD->nameBuf[t + qname.localName->length] = '\0';
		}
		else
		{
			memcpy(appD->nameBuf, qname.localName->str, qname.localName->length);
			appD->nameBuf[qname.localName->length] = '\0';
		}
		push(&(appD->stack), createElement(appD->nameBuf));
		if(appD->unclosedElement)
			fprintf(outfile, ">\n");
		fprintf(outfile, "<%s", appD->nameBuf);

		if(prxHit == 0)
		{
			sprintf(appD->nameBuf, " xmlns:p%d=\"", prefixIndex);
			fprintf(outfile, "%s", appD->nameBuf);
			fprintf(outfile, "%.*s", qname.uri->length, qname.uri->str);
			fprintf(outfile, "\"");
		}

		appD->unclosedElement = 1;
	}

	return EXIP_OK;
}

static errorCode sample_endElement(void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
		fprintf(outfile, "EE\n");
	else if(appD->outputFormat == OUT_XML)
	{
		struct element* el;

		if(appD->unclosedElement)
			fprintf(outfile, ">\n");
		appD->unclosedElement = 0;
		el = pop(&(appD->stack));
		fprintf(outfile, "</%s>\n", el->name);
		destroyElement(el);
	}

	return EXIP_OK;
}

static errorCode sample_attribute(QName qname, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
	{
		fprintf(outfile, "AT ");
		fprintf(outfile, "%.*s", qname.uri->length, qname.uri->str);
		fprintf(outfile, " ");
		fprintf(outfile, "%.*s", qname.localName->length, qname.localName->str);
		fprintf(outfile, "=\"");
	}
	else if(appD->outputFormat == OUT_XML)
	{
		fprintf(outfile, " ");
		if(!isStringEmpty(qname.uri))
		{
			fprintf(outfile, "%.*s", qname.uri->length, qname.uri->str);
			fprintf(outfile, ":");
		}
		fprintf(outfile, "%.*s", qname.localName->length, qname.localName->str);
		fprintf(outfile, "=\"");
	}
	appD->expectAttributeData = 1;

	return EXIP_OK;
}

static errorCode sample_stringData(const String value, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			fprintf(outfile, "%.*s", value.length, value.str);
			fprintf(outfile, "\"\n");
			appD->expectAttributeData = 0;
		}
		else
		{
			fprintf(outfile, "CH ");
			fprintf(outfile, "%.*s", value.length, value.str);
			fprintf(outfile, "\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			fprintf(outfile, "%.*s", value.length, value.str);
			fprintf(outfile, "\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				fprintf(outfile, ">");
			appD->unclosedElement = 0;
			fprintf(outfile, "%.*s", value.length, value.str);
		}
	}

	return EXIP_OK;
}

static errorCode sample_decimalData(Decimal value, void* app_data)
{
	return sample_floatData(value, app_data);
}

static errorCode sample_intData(Integer int_val, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	char tmp_buf[30];
	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			sprintf(tmp_buf, "%lld", (long long int) int_val);
			fprintf(outfile, "%s", tmp_buf);
			fprintf(outfile, "\"\n");
			appD->expectAttributeData = 0;
		}
		else
		{
			fprintf(outfile, "CH ");
			sprintf(tmp_buf, "%lld", (long long int) int_val);
			fprintf(outfile, "%s", tmp_buf);
			fprintf(outfile, "\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			sprintf(tmp_buf, "%lld", (long long int) int_val);
			fprintf(outfile, "%s", tmp_buf);
			fprintf(outfile, "\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				fprintf(outfile, ">");
			appD->unclosedElement = 0;
			sprintf(tmp_buf, "%lld", (long long int) int_val);
			fprintf(outfile, "%s", tmp_buf);
		}
	}

	return EXIP_OK;
}

static errorCode sample_booleanData(boolean bool_val, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;

	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			if(bool_val)
				fprintf(outfile, "true\"\n");
			else
				fprintf(outfile, "false\"\n");

			appD->expectAttributeData = 0;
		}
		else
		{
			fprintf(outfile, "CH ");
			if(bool_val)
				fprintf(outfile, "true\n");
			else
				fprintf(outfile, "false\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			if(bool_val)
				fprintf(outfile, "true\"");
			else
				fprintf(outfile, "false\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				fprintf(outfile, ">");
			appD->unclosedElement = 0;

			if(bool_val)
				fprintf(outfile, "true");
			else
				fprintf(outfile, "false");
		}
	}

	return EXIP_OK;
}

static errorCode sample_floatData(Float fl_val, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	char tmp_buf[30];
	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			sprintf(tmp_buf, "%lldE%d", (long long int) fl_val.mantissa, fl_val.exponent);
			fprintf(outfile, "%s", tmp_buf);
			fprintf(outfile, "\"\n");
			appD->expectAttributeData = 0;
		}
		else
		{
			fprintf(outfile, "CH ");
			sprintf(tmp_buf, "%lldE%d", (long long int) fl_val.mantissa, fl_val.exponent);
			fprintf(outfile, "%s", tmp_buf);
			fprintf(outfile, "\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			sprintf(tmp_buf, "%lldE%d", (long long int) fl_val.mantissa, fl_val.exponent);
			fprintf(outfile, "%s", tmp_buf);
			fprintf(outfile, "\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				fprintf(outfile, ">");
			appD->unclosedElement = 0;
			sprintf(tmp_buf, "%lldE%d", (long long int) fl_val.mantissa, fl_val.exponent);
			fprintf(outfile, "%s", tmp_buf);
		}
	}

	return EXIP_OK;
}

static errorCode sample_dateTimeData(EXIPDateTime dt_val, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	char fsecBuf[30];
	int i;

	if(IS_PRESENT(dt_val.presenceMask, FRACT_PRESENCE))
	{
		unsigned int tmpfValue = dt_val.fSecs.value;
		int digitNum = 0;

		fsecBuf[0] = '.';

		while(tmpfValue)
		{
			digitNum++;
			tmpfValue = tmpfValue / 10;
		}
		for(i = 0; i < dt_val.fSecs.offset + 1 - digitNum; i++)
			fsecBuf[1+i] = '0';

		sprintf(fsecBuf + 1 + i, "%d", dt_val.fSecs.value);
	}
	else
	{
		fsecBuf[0] = '\0';
	}

	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			fprintf(outfile, "%04d-%02d-%02dT%02d:%02d:%02d%s", dt_val.dateTime.tm_year + 1900,
					dt_val.dateTime.tm_mon + 1, dt_val.dateTime.tm_mday,
					dt_val.dateTime.tm_hour, dt_val.dateTime.tm_min,
					dt_val.dateTime.tm_sec, fsecBuf);
			fprintf(outfile, "\"\n");
			appD->expectAttributeData = 0;
		}
		else
		{
			fprintf(outfile, "CH ");
			fprintf(outfile, "%04d-%02d-%02dT%02d:%02d:%02d%s", dt_val.dateTime.tm_year + 1900,
					dt_val.dateTime.tm_mon + 1, dt_val.dateTime.tm_mday,
					dt_val.dateTime.tm_hour, dt_val.dateTime.tm_min,
					dt_val.dateTime.tm_sec, fsecBuf);
			fprintf(outfile, "\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			fprintf(outfile, "%04d-%02d-%02dT%02d:%02d:%02d%s", dt_val.dateTime.tm_year + 1900,
					dt_val.dateTime.tm_mon + 1, dt_val.dateTime.tm_mday,
					dt_val.dateTime.tm_hour, dt_val.dateTime.tm_min,
					dt_val.dateTime.tm_sec, fsecBuf);
			fprintf(outfile, "\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				fprintf(outfile, ">");
			appD->unclosedElement = 0;
			fprintf(outfile, "%04d-%02d-%02dT%02d:%02d:%02d%s", dt_val.dateTime.tm_year + 1900,
					dt_val.dateTime.tm_mon + 1, dt_val.dateTime.tm_mday,
					dt_val.dateTime.tm_hour, dt_val.dateTime.tm_min,
					dt_val.dateTime.tm_sec, fsecBuf);
		}
	}

	return EXIP_OK;
}

static errorCode sample_binaryData(const char* binary_val, Index nbytes, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;

	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			fprintf(outfile, "[binary: %d bytes]", (int) nbytes);
			fprintf(outfile, "\"\n");
			appD->expectAttributeData = 0;
		}
		else
		{
			fprintf(outfile, "CH ");
			fprintf(outfile, "[binary: %d bytes]", (int) nbytes);
			fprintf(outfile, "\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			fprintf(outfile, "[binary: %d bytes]", (int) nbytes);
			fprintf(outfile, "\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				fprintf(outfile, ">");
			appD->unclosedElement = 0;
			fprintf(outfile, "[binary: %d bytes]", (int) nbytes);
		}
	}

	return EXIP_OK;
}

static errorCode sample_qnameData(const QName qname, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			fprintf(outfile, "%.*s", qname.uri->length, qname.uri->str);
			fprintf(outfile, ":");
			fprintf(outfile, "%.*s", qname.localName->length, qname.localName->str);
			fprintf(outfile, "\"\n");
			appD->expectAttributeData = 0;
		}
		else
		{
			fprintf(outfile, "QNAME ");
			fprintf(outfile, "%.*s", qname.uri->length, qname.uri->str);
			fprintf(outfile, ":");
			fprintf(outfile, "%.*s", qname.localName->length, qname.localName->str);
			fprintf(outfile, "\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			fprintf(outfile, "%.*s", qname.uri->length, qname.uri->str);
			fprintf(outfile, ":");
			fprintf(outfile, "%.*s", qname.localName->length, qname.localName->str);
			fprintf(outfile, "\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				fprintf(outfile, ">");
			appD->unclosedElement = 0;
			fprintf(outfile, "%.*s", qname.uri->length, qname.uri->str);
			fprintf(outfile, ":");
			fprintf(outfile, "%.*s", qname.localName->length, qname.localName->str);
		}
	}

	return EXIP_OK;
}

// Stuff needed for the OUT_XML Output Format
// ******************************************
static void push(struct element** stack, struct element* el)
{
	if(*stack == NULL)
		*stack = el;
	else
	{
		el->next = *stack;
		*stack = el;
	}
}

static struct element* pop(struct element** stack)
{
	if(*stack == NULL)
		return NULL;
	else
	{
		struct element* result;
		result = *stack;
		*stack = (*stack)->next;
		return result;
	}
}

static struct element* createElement(char* name)
{
	struct element* el;
	el = malloc(sizeof(struct element));
	if(el == NULL)
		exit(1);
	el->next = NULL;
	el->name = malloc(strlen(name)+1);
	if(el->name == NULL)
		exit(1);
	strcpy(el->name, name);
	return el;
}

static void destroyElement(struct element* el)
{
	free(el->name);
	free(el);
}
// ******************************************

static char lookupPrefix(struct appData* aData, String ns, unsigned char* prxHit, unsigned char* prefixIndex)
{
	int i;
	for(i = 0; i < aData->prefixesCount; i++)
	{
		if(stringEqualToAscii(ns, aData->prefixes[i]))
		{
			*prefixIndex = i;
			*prxHit = 1;
			return 0;
		}
	}

	if(aData->prefixesCount == MAX_PREFIXES)
		return 1;
	else
	{
		memcpy(aData->prefixes[aData->prefixesCount], ns.str, ns.length);
		aData->prefixes[aData->prefixesCount][ns.length] = '\0';
		*prefixIndex = aData->prefixesCount;
		aData->prefixesCount += 1;
		*prxHit = 0;
		return 0;
	}
}
