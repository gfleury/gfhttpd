// package name: golang_bridge
package main

import (
	"go_example"
	"net/http"
)

var golang_modules map[string]*http.ServeMux

func init() {
	if golang_modules == nil {
		golang_modules = map[string]*http.ServeMux{
			"go_example": go_example.Mux,
		}
	}
}

func main() {}
