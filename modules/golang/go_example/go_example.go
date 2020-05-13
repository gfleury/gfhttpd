package go_example

import (
	"fmt"
	"html"
	"time"
	"net/http"
)

var Mux = http.NewServeMux()

func init() {
	Mux.HandleFunc("/date", func(w http.ResponseWriter, req *http.Request) {
		currentTime := time.Now()
		fmt.Fprintf(w, "Current Time in String: %s\n", currentTime.String())
	})
	Mux.HandleFunc("/", ServeRoot)
}

func ServeRoot(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusNotFound)
	fmt.Fprintf(w, "Hello, %q %s\n", html.EscapeString(r.URL.Path), r.Header)
}
