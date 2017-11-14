

While there are a lot of file here, there are only two that you need to add to use
these libraries.

For the synchronous version:
- `exosite-lib/include/exosite.h`
- `exosite-lib/src/exosite.c`

For the asynchronous version:
- `exosite-lib/include/exosite_async.h`
- `exosite-lib/src/exosite_async.c`

Then you need to implement the pal files.  Look to `exosite-pal/pal_sync_template`
or `exosite-pal/pal_async_template` to get started.


# Developing

## Unit Tests

```sh
cmake cmake
make
./testrunner/testrunner
```

