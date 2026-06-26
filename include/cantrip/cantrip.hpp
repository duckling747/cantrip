/*
 * cantrip
 * Arena-backed HTML builder
 *
 * Copyright (c) 2026 Arttu Mykkänen
 * Licensed under the MPL-2.0 License (see LICENSE file).
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <type_traits>
#include <memory_resource>
#include <array>


#ifndef CANTRIP_ARENA_SIZE
#define CANTRIP_ARENA_SIZE (128 * 1024)
#endif


namespace cantrip
{
    using string = std::pmr::string;
    inline thread_local std::array<std::byte, CANTRIP_ARENA_SIZE> arena_buffer;
    inline thread_local std::pmr::monotonic_buffer_resource arena_resource(
        arena_buffer.data(),
        arena_buffer.size(),
        std::pmr::new_delete_resource()
    );

    inline string text(std::string_view s) {
        string out(&arena_resource);
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                case '\'': out += "&apos;"; break;
                default: out += c; break;
            }
        }
        return out;
    }

    inline string raw(std::string_view s) { return string(s, &arena_resource); }

    struct AttrNode {
        string data;
    };

    inline AttrNode attr(std::string_view k, std::string_view v) {
        return AttrNode{ " " + string(k, &arena_resource) + "=\"" + text(v) + "\"" };
    }

    struct Result final {
        string pmr_doc;

        explicit Result(string&& doc) : pmr_doc(std::move(doc)) {}

        operator std::string() && {
            std::string final_html(pmr_doc.data(), pmr_doc.size());
            arena_resource.release();
            return final_html;
        }
    };

    /// Builds a complete HTML document.
    ///
    /// Lifetime:
    /// - All intermediate `cantrip::string` values are allocated from a
    ///   thread-local monotonic arena.
    /// - They remain valid only until the returned `Result` is converted to
    ///   `std::string`.
    /// - Do not retain or use `cantrip::string` values after finalization.
    inline Result doctype(std::string_view v = "html") {
        return Result("<!DOCTYPE " + string(v, &arena_resource) + ">");
    }

    inline Result operator+(Result&& lhs, string&& rhs) {
        lhs.pmr_doc += rhs;
        return std::move(lhs);
    }

    inline Result operator+(Result&& lhs, const string& rhs) {
        lhs.pmr_doc += rhs;
        return std::move(lhs);
    }

    struct Range {
        size_t start;
        size_t end;
    };

    inline Range range(size_t start, size_t end) { return Range{start, end}; }

    template<typename F>
    inline string operator|(const Range& r, F&& f) {
        string out(&arena_resource);
        for (size_t i = r.start; i < r.end; ++i) {
            out += f(i);
        }

        return out;
    }

    template<typename F>
    inline string operator|=(const Range& r, F&& f) {
        string out(&arena_resource);
        for (size_t i = r.start; i <= r.end; ++i) {
            out += f(i);
        }

        return out;
    }

    inline string operator&&(bool cond, const string& node) {
        return cond ? node : "";
    }

    template<typename T>
    inline void pack_nodes(string& attrs, string& children, T&& arg) {
        using Decayed = std::decay_t<T>;

        if constexpr (std::is_same_v<Decayed, AttrNode>) {
            attrs += std::forward<T>(arg).data;
        }
        else if constexpr (std::is_convertible_v<Decayed, std::string_view> && !std::is_same_v<Decayed, string>) {
            children += text(std::forward<T>(arg));
        }
        else {
            children += std::forward<T>(arg);
        }
    }

    #define CANTRIP_TAG(name) \
    template<typename... Args> \
    inline string name(Args&&... args) { \
        string attrs (&arena_resource); \
        string children (&arena_resource); \
        (pack_nodes(attrs, children, std::forward<Args>(args)), ...); \
        return "<" #name + attrs + ">" + children + "</" #name ">"; \
    }
    CANTRIP_TAG(html) CANTRIP_TAG(head) CANTRIP_TAG(body)
    CANTRIP_TAG(div) CANTRIP_TAG(span) CANTRIP_TAG(p) CANTRIP_TAG(a)
    CANTRIP_TAG(ul) CANTRIP_TAG(ol) CANTRIP_TAG(li)
    CANTRIP_TAG(h1) CANTRIP_TAG(h2) CANTRIP_TAG(h3) CANTRIP_TAG(h4) CANTRIP_TAG(h5) CANTRIP_TAG(h6)
    CANTRIP_TAG(title) CANTRIP_TAG(section) CANTRIP_TAG(nav) CANTRIP_TAG(header) CANTRIP_TAG(footer) CANTRIP_TAG(main) CANTRIP_TAG(picture)
    CANTRIP_TAG(article) CANTRIP_TAG(aside) CANTRIP_TAG(address)
    CANTRIP_TAG(pre) CANTRIP_TAG(blockquote) CANTRIP_TAG(em) CANTRIP_TAG(strong) CANTRIP_TAG(small) CANTRIP_TAG(s) CANTRIP_TAG(cite) CANTRIP_TAG(q)
    CANTRIP_TAG(dfn) CANTRIP_TAG(abbr) CANTRIP_TAG(data) CANTRIP_TAG(time) CANTRIP_TAG(code) CANTRIP_TAG(var) CANTRIP_TAG(samp) CANTRIP_TAG(kbd)
    CANTRIP_TAG(sub) CANTRIP_TAG(sup) CANTRIP_TAG(i) CANTRIP_TAG(b) CANTRIP_TAG(u) CANTRIP_TAG(mark) CANTRIP_TAG(ruby) CANTRIP_TAG(rt) CANTRIP_TAG(rp)
    CANTRIP_TAG(bdi) CANTRIP_TAG(bdo)
    CANTRIP_TAG(table) CANTRIP_TAG(caption) CANTRIP_TAG(colgroup) CANTRIP_TAG(tbody) CANTRIP_TAG(thead) CANTRIP_TAG(tfoot) CANTRIP_TAG(tr) CANTRIP_TAG(td) CANTRIP_TAG(th)
    CANTRIP_TAG(form) CANTRIP_TAG(label) CANTRIP_TAG(select) CANTRIP_TAG(optgroup) CANTRIP_TAG(option) CANTRIP_TAG(textarea) CANTRIP_TAG(output) CANTRIP_TAG(progress) CANTRIP_TAG(meter) CANTRIP_TAG(fieldset) CANTRIP_TAG(legend) CANTRIP_TAG(button) CANTRIP_TAG(datalist)
    CANTRIP_TAG(video) CANTRIP_TAG(audio) CANTRIP_TAG(iframe) CANTRIP_TAG(object) CANTRIP_TAG(canvas) CANTRIP_TAG(map) CANTRIP_TAG(noscript) CANTRIP_TAG(script)
    CANTRIP_TAG(details) CANTRIP_TAG(summary) CANTRIP_TAG(dialog)
    #undef CANTRIP_TAG

    #define CANTRIP_VOID_TAG(name) \
    template<typename... Args> \
    inline string name(Args&&... args) { \
        string attrs (&arena_resource); \
        string children (&arena_resource); \
        (pack_nodes(attrs, children, std::forward<Args>(args)), ...); \
        return "<" #name + attrs + ">"; \
    }
    CANTRIP_VOID_TAG(link) CANTRIP_VOID_TAG(img) CANTRIP_VOID_TAG(meta) CANTRIP_VOID_TAG(br) CANTRIP_VOID_TAG(hr)
    CANTRIP_VOID_TAG(input) CANTRIP_VOID_TAG(wbr) CANTRIP_VOID_TAG(area) CANTRIP_VOID_TAG(base) CANTRIP_VOID_TAG(col)
    CANTRIP_VOID_TAG(embed) CANTRIP_VOID_TAG(param) CANTRIP_VOID_TAG(source) CANTRIP_VOID_TAG(track)
    #undef CANTRIP_VOID_TAG
}
