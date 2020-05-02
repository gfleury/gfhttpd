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
	"unsafe"
)

//export Go_golang
func Go_golang(hs *C.struct_http_stream, go_cb interface{}) C.int {
	method := C.GoString(hs.request.method)
	url := C.GoString(hs.request.url)
	request, err := http.NewRequest(method, url, nil)
	if err != nil {
		fmt.Println(err.Error())
		return C.int(-1)
	}

	var s *C.struct_headers

	for s = hs.request.headers; s != nil; s = (*C.struct_headers)(unsafe.Pointer(s.hh.next)) {
		name := C.GoString(s.name)
		value := C.GoString(s.value)
		request.Header.Add(name, value)
	}

	responsewriter := &responseWriter{}

	responsewriter.setFd(uintptr(hs.response.fd))
	responsewriter.contentLenght = &hs.response.content_lenght
	responsewriter.char_status = &hs.response.http_status
	responsewriter.size = -1

	defer responsewriter.fd.Close()
	funcx := go_cb.(func(w http.ResponseWriter, r *http.Request))

	funcx(responsewriter, request)
	return C.int(0)
}

func Simple_Example(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusNotFound)
	fmt.Fprintf(w, "Hello, %q %s", html.EscapeString(r.URL.Path), r.Header)
}

//export Get_address
func Get_address() interface{} {
	return Simple_Example
}
