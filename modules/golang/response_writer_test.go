package main

import "C"

import (
	"bufio"
	"net/http"
	"os"
	"syscall"
	"testing"
)

func TestResponseWriterHeader(t *testing.T) {
	w := &responseWriter{}
	w.Header().Set("Content-Type", "text/plain; charset=utf-8")
	w.Header().Set("X-Content-Type-Options", "nosniff")
}

func TestGoExample(t *testing.T) {
	w := &responseWriter{}
	status := C.CString("404")
	cl := C.ulong(0)

	pipeFile := "pipetest"

	os.Remove(pipeFile)
	err := syscall.Mkfifo(pipeFile, 0666)
	if err != nil {
		t.Error(err)
	}
	file, err := os.OpenFile(pipeFile, os.O_RDWR|os.O_CREATE|os.O_APPEND, os.ModeNamedPipe)
	if err != nil {
		t.Error(err)
	}
	defer file.Close()

	w.setFd(file.Fd())
	w.contentLenght = &cl
	w.char_status = &status
	w.size = -1

	r, err := http.NewRequest("GET", "/golang", nil)
	if err != nil {
		t.Error(err)
	}

	handle := golang_modules["go_example"]

	go handle.ServeHTTP(w, r)

	filereader, err := os.OpenFile(pipeFile, os.O_CREATE, os.ModeNamedPipe)
	if err != nil {
		t.Error(err)
	}
	defer filereader.Close()

	reader := bufio.NewReader(filereader)

	line, err := reader.ReadString('\n')
	if err != nil {
		t.Error(err)
	}
	if line != "Hello, \"/golang\" map[]\n" {
		t.Errorf("%s != %s", "Hello, \"/golang\" map[]", line)
	}

}
