use std::{
    error::Error,
    ffi::{CStr, CString},
};

pub unsafe fn call_libc(key: &str) -> Result<String, Box<dyn Error>> {
    if key == "PID" {
        let pid = libc::getpid();

        return Ok(pid.to_string());
    }

    if key == "PPID" {
        let ppid = libc::getppid();

        return Ok(ppid.to_string());
    }

    if key == "PWD" {
        let buf = {
            let mut buf = Vec::with_capacity(4096);

            let ptr = buf.as_mut_ptr() as *mut libc::c_char;

            if libc::getcwd(ptr, buf.capacity()).is_null() {
                panic!("Couldn't get current working directory")
            }

            let c_str = CStr::from_ptr(ptr);
            buf.set_len(c_str.to_bytes().len());

            CString::new(buf)
        }?;

        let cwd = buf.into_string()?;

        return Ok(cwd);
    }

    if key == "RAND" {
        let rand = libc::rand();

        return Ok(rand.to_string());
    }

    Ok("".to_string())
}
