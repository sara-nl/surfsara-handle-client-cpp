# Handle Client
A c++ header only library and an a CLI application.

## Using the CLI tool

### Preparation

```
OPTIONS="--unsecure --port 8003 --key 308_21.T12995_TRAINING_privkey.pem --cert 308_21.T12995_TRAINING_certificate_only.pem "
URL=https://epic4.storage.surfsara.nl:8007/api/handles
PREFIX=21.T12995
```
### Create a resource

```
./handle $OPTIONS create $URL $PREFIX \
    '1={"type":"URL", 
        "data": {"format": "string",
                 "value":"https://ndownloader.figshare.com/files/2292172"}}' \
    '100={"type": "HS_ADMIN",
          "data": {"format": "admin",
                    "value": {"handle": "0.NA/21.T12995",
                              "index": 200,
                              "permissions": "011111110011"}}}'
```

### Get handle information

Get a complete handle record 

```
./handle $OPTIONS get $URL $PREFIX/<UUID> 
```

Get a specific index

```
./handle $OPTIONS get $URL $PREFIX/<UUID> <INDEX>
```

Get a specific field in the index record (e.g. the type)
```
./handle $OPTIONS get $URL $PREFIX/<UUID> <INDEX>/type
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

./handle update $URL $PREFIX/$UUID 1/data/value="https://ndownloader.figshare.com/files/2292172" \
                           100/data/value/index=delete \
                           '101={"type": "URL",
                                 "data": {"format": "string",
                                          "value": "https://...."}}'
```

Remove an index from resource
```
UUID="...."

./handle update $URL $PREFIX/$UUID 100=delete
```

### Remove a resource

```
./handle delete $URL $PREFIX/$UUID
```
