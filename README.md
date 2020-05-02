# gfhttpd

Main goal of the project is to learn about the Bazel building system with different toolchains/languages.
Creating a complex multi-language project with inter-linking between C/Golang and Rust.
So, an http server supporting HTTP protocol verions 2 and 3, core loop using libevent and extensible in Golang and Rust.


## HTTP 2

* uses nghttp <https://github.com/nghttp2/nghttp2>

## HTTP 3

* uses quiche <https://github.com/cloudflare/quiche>

## Golang

```golang
func(w http.ResponseWriter, r *http.Request) {
    fmt.Fprintf(w, "Hello, %q", html.EscapeString(r.URL.Path))
}
```

## Rust

* <https://hyper.rs/>

## Build

```bash
bazel build //main:gfhttpd
```
