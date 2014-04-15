#!/bin/bash

NUMBER_OF_RUNS=100

echo "This script runs the evaluation experiment"
echo "It decodes 3 instances for each of the 5 XML schemas by EXIP, EXIFicient and OpenEXI"
echo "Each instance is decoded $NUMBER_OF_RUNS times"
echo "The output is a CSV file evaluation.csv with the following comlumns (all in nanoseconds):"
echo "[OpenEXI netconf gr] ; [OpenEXI netconf 01/02/03] ; [EXIFicient netconf gr] ; [EXIFicient netconf 01/02/03] ; [exip netconf gr] ; [exip netconf 01/02/03] ... (the other schema files in the same order)"
printf "\n"

#EXI processors binaries
EXIP="exip-0.5.3/bin/examples/exipd"
EXIFicient="./run-exificient.sh"
OpenEXI="./run-OpenEXIDecode.sh"

#XML Schemas
SchemaNetconf="../Schemas/netconf.xsd"
SchemaSenML="../Schemas/SenML.xsd"
SchemaSEP="../Schemas/sep.xsd"
SchemaOPC="../Schemas/OPC-UA-Types.xsd"
SchemaXMLSchema="../Schemas/XMLSchema.xsd"

#XML schemas EXI'd
SchemaNetconfEXI="../Schemas/netconf-xsd.exi"
SchemaSenMLEXI="../Schemas/SenML-xsd.exi"
SchemaSEPEXI="../Schemas/sep-xsd.exi"
SchemaOPCEXI="../Schemas/OPC-UA-Types-xsd.exi"
SchemaXMLSchemaEXI="../Schemas/XMLSchema-xsd.exi"

SchemaXMLSchemaXMLXSDEXI="../Schemas/xml-xsd.exi"

#EXI instances
NetConf[1]="../EXI-Output/netconf-01.exi"
NetConf[2]="../EXI-Output/netconf-02.exi"
NetConf[3]="../EXI-Output/netconf-03.exi"

SenML[1]="../EXI-Output/SenML-01.exi"
SenML[2]="../EXI-Output/SenML-02.exi"
SenML[3]="../EXI-Output/SenML-03.exi"

SEP2[1]="../EXI-Output/SEP2-01.exi"
SEP2[2]="../EXI-Output/SEP2-02.exi"
SEP2[3]="../EXI-Output/SEP2-03.exi"

OPCUA[1]="../EXI-Output/OPC-UA-Types-01.exi"
OPCUA[2]="../EXI-Output/OPC-UA-Types-02.exi"
OPCUA[3]="../EXI-Output/OPC-UA-Types-03.exi"

XMLSchema[1]="../Schemas/no-schema-id/netconf-noid.exi"
XMLSchema[2]="../Schemas/no-schema-id/SenML-noid.exi"
XMLSchema[3]="../Schemas/no-schema-id/sep-noid.exi"

if [ ! -f "$EXIP" ]
  then
    echo "exipd binary does not exist in exip-0.5.3/bin/examples/"
    exit 1
fi

if [ ! -f "$EXIFicient" ]
  then
    echo "EXIFicient binary does not exist in EXIFicient/"
    exit 1
fi

if [ ! -f "$OpenEXI" ]
  then
    echo "OpenEXI binary does not exist in OpenEXI/"
    exit 1
fi

#Clear content of evaluation.csv
echo "" > evaluation.csv
#Append the column headers to evaluation.csv
echo "[OpenEXI netconf gr] ; [OpenEXI netconf 01/02/03] ; [EXIFicient netconf gr] ; [EXIFicient netconf 01/02/03] ; [exip netconf gr] ; [exip netconf 01/02/03] ; [OpenEXI SenML gr] ; [OpenEXI SenML 01/02/03] ; [EXIFicient SenML gr] ; [EXIFicient SenML 01/02/03] ; [exip SenML gr] ; [exip SenML 01/02/03] ; [OpenEXI SEP2 gr] ; [OpenEXI SEP2 01/02/03] ; [EXIFicient SEP2 gr] ; [EXIFicient SEP2 01/02/03] ; [exip SEP2 gr] ; [exip SEP2 01/02/03] ; [OpenEXI OPCUA gr] ; [OpenEXI OPCUA 01/02/03] ; [EXIFicient OPCUA gr] ; [EXIFicient OPCUA 01/02/03] ; [exip OPCUA gr] ; [exip OPCUA 01/02/03] ; [OpenEXI Schema gr] ; [OpenEXI Schema 01/02/03] ; [EXIFicient Schema gr] ; [EXIFicient Schema 01/02/03] ; [exip Schema gr] ; [exip Schema 01/02/03]" >> evaluation.csv


for (( i=1; i < $NUMBER_OF_RUNS + 1; i++ ))
do
  echo "Run #$i"

  for j in 1 2 3
  do
      echo "INSTANCE $j"

      #--OpenEXI netconf 0$j 
      $OpenEXI $SchemaNetconf ${NetConf[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--EXIFicient netconf 0$j
      $EXIFicient $SchemaNetconf ${NetConf[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--exip netconf 0$j
      $EXIP -xml -schema=$SchemaNetconfEXI,$SchemaXMLSchemaXMLXSDEXI ${NetConf[j]} >> evaluation.csv
      printf " ; " >> evaluation.csv
      
      #--OpenEXI SenML 0$j
      $OpenEXI $SchemaSenML ${SenML[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--EXIFicient SenML 0$j
      $EXIFicient $SchemaSenML ${SenML[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--exip SenML 0$j
      $EXIP -xml -schema=$SchemaSenMLEXI ${SenML[j]} >> evaluation.csv
      printf " ; " >> evaluation.csv

      #--OpenEXI SEP2 0$j
      $OpenEXI $SchemaSEP ${SEP2[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--EXIFicient SEP2 0$j
      $EXIFicient $SchemaSEP ${SEP2[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--exip SEP2 0$j
      $EXIP -xml -schema=$SchemaSEPEXI ${SEP2[j]} >> evaluation.csv
      printf " ; " >> evaluation.csv

      #--OpenEXI OPCUA 0$j
      $OpenEXI $SchemaOPC ${OPCUA[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--EXIFicient OPCUA 0$j
      $EXIFicient $SchemaOPC  ${OPCUA[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--exip OPCUA 0$j
      $EXIP -xml -schema=$SchemaOPCEXI  ${OPCUA[j]} >> evaluation.csv
      printf " ; " >> evaluation.csv

      #--OpenEXI XML Schema 0$j
      $OpenEXI $SchemaXMLSchema ${XMLSchema[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--EXIFicient XML Schema 0$j
      $EXIFicient $SchemaXMLSchema ${XMLSchema[j]} $1 >> evaluation.csv
      printf " ; " >> evaluation.csv
      #--exip XML Schema 0$j
      $EXIP -xml -schema=$SchemaXMLSchemaEXI,$SchemaXMLSchemaXMLXSDEXI ${XMLSchema[j]} >> evaluation.csv

      printf "\n" >> evaluation.csv
  done

done

echo "Complete!"
