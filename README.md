Redis Zstandard Compressed Values Module
========================================

Magically make your Redis data smaller with Zstandard compression.

Often when running large-scale Redis deployments, I find myself wishing I magically had more memory, but didn't need to pay anything for it. Sadly that's never going to happen, but by compressing the data stored in Redis, we *can* often trade CPU for memory - burning extra CPU cycles to compress data before its stored in Redis, so that less memory is used per value.

The biggest challenge with compressing values in Redis is that often the compression ratios achieved when you're storing lots of little keys can be really low. Most compression schemes (like zlib) really only start to achieve good compression once they have a large amount of data to work with. That means that only the biggest keys can really benefit.

Recently, Facebook announced an awesome new compression library called Zstandard - a next generation compression library to replace zlib/xz/7zip and friends. Not only boasting great performance, it also allows you to pre-compute a dictionary to use for compression. That allows even small keys to benefit from compression, and makes it the perfect library to use for storing compressed data in Redis.

This module provides 4 new commands - two for getting and setting compressed values, and two for getting and setting compressed values using a pre-computed Zstandard dictionary.
