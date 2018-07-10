# Handle Client
A c++ header only library and an a CLI application.

## Using the CLI tool

### Preparation

```
URL=https://epic4.storage.surfsara.nl:8007/api/handles/21.T12995
```
### Create a resource

```
./handle create $URL auto \
    1:'{"type":"URL", 
        "data": {"format": "string",
                 "value":"https://ndownloader.figshare.com/files/2292172"}}' \
    100:'{"type": "HS_ADMIN",
          "data": {"format": "admin",
                    "value": {"handle": "0.NA/21.T12995",
                              "index": 200,
                              "permissions": "011111110011"}}}'
```


### Update a resource 
Update one value in the tree:
```
UUID="...."

./handle update $URL $UUID 1/data/value="https://ndownloader.figshare.com/files/2292172"
```
Modify multiple values in the tree (update a value, remove an entry of a subobject, adding a new index):
```
UUID="...."

./handle update $URL $UUID 1/data/value="https://ndownloader.figshare.com/files/2292172" \
                           100/data/value/index=delete \
                           101='{"type": "URL",
                                 "data": {"format": "string",
                                          "value": "https://...."}}'
```

Remove an index from resource
```
UUID="...."

./handle update $URL $UUID 100=delete
```

### Remove a resource

```
./handle delete $URL $UUID
```
