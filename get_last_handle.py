#!/usr/bin/env python
import os.path
import json

if os.path.isfile('last_handle.json'):
    with open('last_handle.json') as f:
        data = json.load(f)
        print(str(data.get('handle')))
else:
    print("")
