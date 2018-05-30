package main  
/* 
#cgo LDFLAGS: -lrt
#include <stdlib.h>  
#include "shmread.h"  
*/
import "C"  
import "unsafe"  
import "fmt"  
  
func read(filename string) string {  
    f := C.CString(filename)  
    defer C.free(unsafe.Pointer(f))  
    s := C.readshmm(f)  
    defer C.free(unsafe.Pointer(s))  
    return C.GoString(s)  
}  
  
func main() {  
    fmt.Println(read("mqtt_shm_test"))  
}  