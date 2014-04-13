package openexi.sample;

import java.io.FileNotFoundException;
import java.io.IOException;

import javax.xml.transform.TransformerConfigurationException;

import org.openexi.proc.common.EXIOptionsException;
import org.openexi.scomp.EXISchemaFactoryException;
import org.xml.sax.SAXException;

public final class OpenEXIDecode {

	static String OUTPUT_FILE = "outputOpenEXI.xml";
	static String INPUT_FILE = "./sample-data/Profile/indexed-01.exi%%vi%%%%";
	static String SCHEMA = "./sample-data/Profile/indexed.xsd";

	public static void main(String[] args) throws Exception {
		if (args.length > 1) {
			OpenEXIDecode.INPUT_FILE = args[0];
			OpenEXIDecode.SCHEMA = args[1];
//			System.out.println("IN: " + OpenEXIDecode.INPUT_FILE + " SCHEM: " + OpenEXIDecode.SCHEMA);
		} else {
			System.out.println("# EXIficient Sample, no input files specified");
			System.out.println("Usage: " + OpenEXIDecode.class.getName()
					+ " xmlFile xsdFile [NumberOfRuns]");
			System.exit(1);
		}
		
		// Instantiate the example class DecodeEXI
        DecodeEXI decode = new DecodeEXI();
        
        try {
            decode.decodeEXI(INPUT_FILE, OUTPUT_FILE, "bitPacked", false, false, false, false, false,
            	1000000, -1, -1, SCHEMA, "", false, "XSD", false, "", false);
        } catch (FileNotFoundException f) {
        	f.printStackTrace();        
        } catch (IOException f) {
        	f.printStackTrace();  
        } catch (TransformerConfigurationException f) {
        	f.printStackTrace();  
        } catch (EXIOptionsException f) {
        	f.printStackTrace();  
        } catch (SAXException f) {
        	f.printStackTrace();  
        } catch (ClassNotFoundException f) {
        	f.printStackTrace();          
        } catch (EXISchemaFactoryException f) {
        	f.printStackTrace();  
        }
	}
}
