package main

import "C"

import (
	"fmt"

	"libevent"
)

//export send_document_cb
func send_document_cb(c_req uintptr, arg *interface{}) {
	req := libevent.SwigcptrStruct_SS_evhttp_request(c_req)

	req_type := libevent.Evhttp_request_get_command(req)
	if req_type != libevent.EVHTTP_REQ_GET {
		dump_request_cb(c_req, arg)
		return
	}

	uri := libevent.Evhttp_request_get_uri(req)
	fmt.Printf("Got a GET request for <%s>\n", uri)

	/* Decode the URI */
	decoded := libevent.Evhttp_uri_parse(uri)
	if decoded == nil {
		fmt.Printf("It's not a good URI. Sending BADREQUEST\n")
		libevent.Evhttp_send_error(req, libevent.HTTP_BADREQUEST, "")
		return
	}

	/* Let's see what path the user asked for. */
	path := libevent.Evhttp_uri_get_path(decoded)
	if path == "" {
		path = "/"
	}

	libevent.Evhttp_send_error(req, libevent.HTTP_OK, "Hello World!")
	return
}

//export dump_request_cb
func dump_request_cb(c_req uintptr, arg *interface{}) {
	return
}

func main() {}
