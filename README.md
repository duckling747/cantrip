# Cantrip

**Cantrip** is a hyper-performance, type-safe, and closure-based C++20 HTML template engine. It is explicitly designed for high-performance asynchronous web servers (such as Drogon or Crow) where global heap allocations (`malloc`/`new`) must be completely eliminated during page rendering.

Inspired by the ergonomics of the *Maud* library in Rust, Cantrip brings a clean, declarative HTML DSL to modern C++20.

## Features

- **Zero-Allocation Rendering:** Dynamic HTML trees, loops, and conditional statements run with zero heap allocations, leveraging a fast `thread_local` PMR arena buffer.
- **Automated RAII Memory Management:** The arena memory pointer is automatically reset back to the beginning in O(1) time the exact moment the rendering lifecycle completes and the document is converted to a standard `std::string`.
- **Clean & Idiomatic DSL:** No messy buffer references or manual cleanup calls polluting your handlers.
- **Automatic HTML Escaping:** Raw text literals are automatically escaped to prevent XSS vulnerabilities, unless explicitly wrapped in a `raw()` marker.

## Project Structure

```text
cantrip/
├── CMakeLists.txt
├── LICENSE
├── README.md
└── include/
    └── cantrip/
        └── cantrip.hpp
```

## Usage Example

Cantrip is entirely `header-only`. You can stitch together complex HTML5 layouts seamlessly using nested functions and the custom pipe operator (`|`) for repetitions:

```cpp
#include <cantrip/cantrip.hpp>
#include <string>

std::string render_index_page()
{
    using namespace cantrip;

    // The HTML tree is constructed entirely inside the thread-local stack arena
    auto html_doc = html(
        attr("lang", "en"),
        head(title("My Home Page")),
        body(
            h1("Welcome to the Realm"),
            p("This page was generated directly inside the CPU's L1/L2 cache."),
            
            // Repeat elements seamlessly without dynamic vectors or heap thrashing
            ul(
                range(1, 5) | [](int i) {
                    return li("Item number " + std::to_string(i));
                }
            )
        )
    );

    // Returning implicitly converts the Result to a std::string for the web framework.
    // This triggers the RAII reset on the underlying arena for the next HTTP request!
    return doctype() + html_doc;
}
```

## CMake Integration

You can integrate Cantrip into your existing project locally or pull it directly from GitHub using CMake's modern `FetchContent` API:

```cmake
include(FetchContent)

FetchContent_Declare(
    cantrip
    GIT_REPOSITORY https://github.com/duckling747/cantrip.git
    GIT_TAG        v0.0.8
)
FetchContent_MakeAvailable(cantrip)

# Linking automatically propagates the C++20 flag and include directories
target_link_libraries(my_web_server PRIVATE cantrip)
```

## License

This source code form is subject to the terms of the **Mozilla Public License 2.0 (MPL-2.0)**. 
