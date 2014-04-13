import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.sax.SAXSource;
import javax.xml.transform.stream.StreamResult;

import org.xml.sax.InputSource;
import org.xml.sax.XMLReader;

import com.siemens.ct.exi.CodingMode;
import com.siemens.ct.exi.EXIFactory;
import com.siemens.ct.exi.EncodingOptions;
import com.siemens.ct.exi.FidelityOptions;
import com.siemens.ct.exi.GrammarFactory;
import com.siemens.ct.exi.api.sax.EXISource;
import com.siemens.ct.exi.grammars.Grammars;
import com.siemens.ct.exi.helpers.DefaultEXIFactory;

public final class EXIficientDecode {

	static String OUTPUT_FILE = "outputEXIFicient.xml";
	static String INPUT_FILE = "./sample-data/Profile/indexed-01.exi%%vi%%%%";
	static String SCHEMA = "./sample-data/Profile/indexed.xsd";

	public static void main(String[] args) throws Exception {
		if (args.length > 1) {
			EXIficientDecode.INPUT_FILE = args[0];
			EXIficientDecode.SCHEMA = args[1];
//			System.out.println("IN: " + EXIficientDecode.INPUT_FILE + " SCHEM: " + EXIficientDecode.SCHEMA);
		} else {
			System.out.println("# EXIficient Sample, no input files specified");
			System.out.println("Usage: " + EXIficientDecode.class.getName()
					+ " xmlFile xsdFile [NumberOfRuns]");
			System.exit(1);
		}
		
		File xsdFile = new File(EXIficientDecode.SCHEMA);
		if (xsdFile.exists())
		{
//			System.out.println("SchemaInformed: \n");
			long startTime = System.nanoTime();
			
			EXIFactory exiFactory = DefaultEXIFactory.newInstance();
//			exiFactory.setFidelityOptions(FidelityOptions.createStrict());
//			exiFactory.setMaximumNumberOfEvolvingBuiltInElementGrammars(0);
//			exiFactory.setMaximumNumberOfBuiltInProductions(0);
//			exiFactory.setLocalValuePartitions(false);
			
			GrammarFactory grammarFactory = GrammarFactory.newInstance();
			Grammars g = grammarFactory.createGrammars(EXIficientDecode.SCHEMA);
			exiFactory.setGrammars(g);
			
			long endTimeSchema = System.nanoTime();
			
			EXISource saxSource = new EXISource(exiFactory);
			XMLReader xmlReader = saxSource.getXMLReader();
			TransformerFactory tf = TransformerFactory.newInstance();
			Transformer transformer = tf.newTransformer();

			InputStream exiIS = new FileInputStream(EXIficientDecode.INPUT_FILE);
			SAXSource exiSource = new SAXSource(new InputSource(exiIS));
			exiSource.setXMLReader(xmlReader);

			OutputStream os = new FileOutputStream(EXIficientDecode.OUTPUT_FILE);
			transformer.transform(exiSource, new StreamResult(os));
			os.close();
			
			long endTimeAll = System.nanoTime();
			
			long durationSchema = endTimeSchema - startTime;
			long durationALL = endTimeAll - startTime;
			
			System.out.print(durationSchema + " ; ");
			System.out.print(durationALL);
		}
		else
		{
			System.out.println("Schema-less: \n");
			SAXSource exiSource = new EXISource();
			XMLReader exiReader = exiSource.getXMLReader();
			TransformerFactory tf = TransformerFactory.newInstance();
			Transformer transformer = tf.newTransformer();

			InputStream exiIS = new FileInputStream(EXIficientDecode.INPUT_FILE);
			SAXSource saxSrc = new SAXSource(new InputSource(exiIS));
			saxSrc.setXMLReader(exiReader);

			OutputStream os = new FileOutputStream(EXIficientDecode.OUTPUT_FILE);
			transformer.transform(saxSrc, new StreamResult(os));
			os.close();
		}
	}

}
