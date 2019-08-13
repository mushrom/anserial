A serializer for storing s-expression-like data structures.

requirements: relatively new c++ compiler (c++11 should do), GNU make

building: `make` to build everything, `make libs` to build only the library, `make tests` to build and run tests.

### Features:
- pretty fast, simple format. Optimized for generation/parsing speed.
- each entry being 8 bytes and referring to only the parent
  means you can stop reading after any 8-byte segment and have a valid
  tree

### Caveats:
- symbols are stored as 32-bit hashes, collisions are inevitable eventually
  - could use map symbols to IDs in the order introduced, then
    use the symbol map to rebuild on the other side
  - I doubt it'll really be an issue for my use cases though, where hash collisions
    are only significant if they happen within the same container, so I'm in no rush
    to ~~complicate~~ fix things now.
- not very space efficient, always uses 8 bytes even just to store a 4-byte int.
- strings are especially inefficient, using an int entry for each character, which means
  about 7 bytes of wasted space per character. I sleep.
- 30 bits for parent IDs means there's an implicit limit of 8GB for generated
  output (~1 billion entries * 8 bytes).
