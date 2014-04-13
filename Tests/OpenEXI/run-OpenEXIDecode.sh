#!/bin/bash

java -classpath OpenEXI/nagasena.jar:OpenEXI/xercesImpl.jar:OpenEXI/xml-apis.jar:OpenEXI:OpenEXI/openexi/sample openexi/sample/OpenEXIDecode $2 $1
