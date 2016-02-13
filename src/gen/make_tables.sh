#!/usr/bin/env bash

set -o errexit
set -o pipefail

cat to_download.txt | while read url; do
   curl $url > $(basename $url)
done

ruby data_generator.rb < UnicodeData.txt > ../data.ic

cat to_download.txt | while read url; do
   rm $(basename $url)
done
