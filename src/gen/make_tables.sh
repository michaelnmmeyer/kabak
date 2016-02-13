#!/usr/bin/env bash

set -o errexit
set -o pipefail

cat to_download.txt | while read url; do
   curl $url > $(basename $url)
done

python3 custom_unidata.py < UnicodeData.txt | ruby data_generator.rb > ../data.ic

cat to_download.txt | while read url; do
   rm $(basename $url)
done
