import hashlib

class HeaderChecker:
    '''Performs checks over the provided C++ headers.'''

    def __init__(self, headers, cache_path):
        self.cache_path = cache_path
        self.headers_cache = self.__read_cache()
        (self.to_add, self.to_remove, self.to_modify) = self.__filter_headers(headers)
        
    def __del__(self):
        for key in self.to_add:
            self.headers_cache[key] = self.to_add[key]
        for key in self.to_remove:
            self.headers_cache.pop(key)
        for key in self.to_modify:
            self.headers_cache[key] = self.to_modify[key]
        self.__write_cache()

    def result(self):
        '''Returns a tuple (additions, removals, changes) of lists resulting from the headers check.'''
        return (self.to_add, self.to_remove, self.to_modify)

    def __read_cache(self):
        '''Reads the cache file (if present) and populates a dictionary with {filename, content_hash}.'''
        # Cache format: filename=content_hash, one per line, path being relative to the bomb_engine folder
        # no need for fancier setups
        cache_dict = {}
        try:
            lines = open(self.cache_path).readlines()
            for index, line in enumerate(lines):
                substrings = line.strip().split("=")
                if len(substrings) != 2:
                    # line does not conform to the standard, report error and show what triggered it
                    print(f"format error at line {index}:   {line}")
                    continue
                # add the cached checksums to the dictionary
                cache_dict[substrings[0]] = substrings[1]
        except:
            # no need to propagate it, simply inform that the file isn't there
            print("No header cache found.")
        return cache_dict

    def __filter_headers(self, headers):
        '''Retrieves the headers that need to be updated, added or removed'''
        # retrieve a copy of the current cache to avoid invalidating the current one
        cache_copy = self.headers_cache.copy()
        # we also want to carry out the hashes for these two here
        to_add = {}
        to_change = {}
        for header in headers:
            try:
                content = open(header).read().encode("utf-8")
                content_hash = hashlib.md5(content).hexdigest()
                trimmed = self.__trim_header(header)
                # find in cache and put in one of the three lists
                if trimmed in cache_copy:
                    # the file is in the cache
                    if cache_copy[trimmed] != content_hash:
                        # add it to the "to change" dictionary
                        print(f"cached: {cache_copy[trimmed]}, new hash: {content_hash}")
                        to_change[trimmed] = content_hash
                    # remove it from local cache
                    cache_copy.pop(trimmed)
                else:
                    # not found, it needs to be added
                    to_add[trimmed] = content_hash
            except OSError as e:
                # no need to propagate it, simply inform that the file isn't there
                print(e)
        # what remains in the cache goes into the to_remove list because it isn't there any more
        # no need for the hash because it will be removed from cache
        to_remove = [key for key in cache_copy]
        return (to_add, to_remove, to_change)

    def __trim_header(self, header: str):
        '''Gets rid of the absolute path to have a relative path instead (relative to bomb_engine dir).
           It is necessary to have a key in the cache that works independently from the engine location.'''
        # might break if the repository is positioned inside another "bomb_engine" directory, but I won't bother now
        engine_dir = "/bomb_engine/"
        trimmed_header = header[header.find(
            engine_dir) + len(engine_dir):]
        return trimmed_header

    def __write_cache(self):
        '''Writes the updated cache to disk'''
        lines = [f"{key}={self.headers_cache[key]}\n" for key in self.headers_cache]
        with open(self.cache_path, "w") as cache:
            cache.writelines(lines)