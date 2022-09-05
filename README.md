# inplace
Simple tool to copy a file inplace writing only modified blocks.

Usage:
```inplace dst_file <src_file```

Both files should already exist and be of the same size. The tool
doesn't make sense in a general case and ```cp src_file dst_file```
should accomplish the same and more. The only useful situation it
can be used is a filesystem which supports snapshots, like ZFS.
In this case it allows to write only changed data, thus saving
storage space allocated in snapshots of the original data.

The size of block is currently hardcoded to be 128k and can be easily changed in the source :)
