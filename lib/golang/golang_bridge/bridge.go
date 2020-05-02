// package name: golang_bridge
package main

// #cgo CFLAGS: -g -Wall
// #include <stdlib.h>
// #include "http_stream/http_stream.h"
import "C"

import (
	"fmt"
	"html"
	"net/http"
)

//export Go_golang
func Go_golang(hs *C.struct_http_stream, go_cb func(interface{}, interface{})) C.int {
	method := C.GoString(hs.request.method)
	url := C.GoString(hs.request.url)
	request, err := http.NewRequest(method, url, nil)
	if err != nil {
		fmt.Println(err.Error())
		return C.int(-1)
	}

	responsewriter := &responseWriter{}

	responsewriter.setFd(uintptr(hs.response.fd))
	responsewriter.contentLenght = &hs.response.content_lenght

	defer responsewriter.fd.Close()

	Simple_Example(responsewriter, request)
	return C.int(0)
}

func Simple_Example(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "Hello, %q", html.EscapeString(r.URL.Path))
}
