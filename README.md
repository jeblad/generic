# generic

An extension for the Heuristic Reasoning Agent (hera) framework that provides the generic functionality.

# Development

### Building

To configure and build the extension as a shared module (`.so`):

```sh
cmake -B build
cmake --build build
```

The plugin will be generated at `build/lib/generic.so`.

### Testing

The project includes unit tests to verify API level compatibility and the internal build logic:

```sh
( cd build && ctest --output-on-failure )
```

### Coding Standards

Development is inspired by:

- **NASA JPL's Power of Ten**: Rules for developing safety-critical code.
- **Google C++ Style Guide**: For consistency in layout and naming conventions.

## Acknowledgements

*Created with assistance from AI tools (Gemini 2.5, 3.0, and 3.1, in both Flash and Pro versions) across all parts of this work.*

This project was developed independently, with no external financial or institutional support other than the AI tools mentioned. The views and conclusions contained herein are those of the author(s) and should not be interpreted as representing the official policies or endorsements, either expressed or implied, of any external agency or entity.
