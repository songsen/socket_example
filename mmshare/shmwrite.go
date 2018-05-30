package main  
  
// #include <stdlib.h>  
// #include "wrapper.c"  
import "C"  
import "unsafe"  
  
import (  
    "log"  
    "time"  
)  
  
type errorString struct {  
    s string  
}  
  
func (e *errorString) Error() string {  
    return e.s  
}  
  
func open(file string) (int, error) {  
    f := C.CString(file)  
    defer C.free(unsafe.Pointer(f))  
    r := int(C.my_shm_open(f, C.int(1)))  
    if r == -1 {  
        return 0, &errorString{"error"}  
    }  
    return r, nil  
}  
  
func update(shm_id int, content string) error {  
    c := C.CString(content)  
    defer C.free(unsafe.Pointer(c))  
    r := int(C.my_shm_update(C.int(shm_id), c))  
    if r == -1 {  
        return &errorString{"update error"}  
    }  
    return nil  
}  
  
func close(shm_id int) error {  
    C.my_shm_close(C.int(shm_id))  
    return nil  
}  
  
func main() {  
    id, err := open("mqtt_shm_test")  
    if err != nil {  
        log.Fatal(err)  
    }  
    defer close(id)  
    err = update(id, "hello world")  
    if err != nil {  
        log.Fatal(err)  
    }  
    time.Sleep(1e9 * 100)  
}  