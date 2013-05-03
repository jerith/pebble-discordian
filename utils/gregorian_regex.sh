#!/bin/bash

days="Monday Tuesday Wednesday Thursday Friday Saturday Sunday"
months="January February March April May June July August September October November December"
digits="0123456789"
other=""

$(dirname $0)/font_chars.py "${days}${months}${digits}${other}"
