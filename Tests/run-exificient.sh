#!/bin/bash

java -classpath EXIFicient/exificient.jar:EXIFicient/xercesImpl.jar:EXIFicient EXIficientDecode $1 $2 $3
