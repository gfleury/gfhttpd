package go_example

import (
	"fmt"
	"html"
	"net/http"
)

type httpHandler struct {
}

var Mux = http.NewServeMux()

func init() {
	handle := httpHandler{}
	Mux.Handle("/golang", &handle)
}

func (hh *httpHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusNotFound)
	fmt.Fprintf(w, "Hello, %q %s\n", html.EscapeString(r.URL.Path), r.Header)
}
