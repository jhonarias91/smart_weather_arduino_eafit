package com.jhonarias91.com.xbeereceiver;

import org.apache.juli.logging.Log;
import org.apache.juli.logging.LogFactory;
import org.springframework.web.bind.annotation.*;

@RestController
public class ApiRestController {

    Log log = LogFactory.getLog(ApiRestController.class);

    @GetMapping("/xbee")
    public String getData(@RequestParam String msg){
        log.info( "Receive " + msg);
        return "Receive " + msg;
    }

    @PostMapping("/xbee")
    public String receive(@RequestBody DefaultBody body){
        log.info( "Receive " + body.toString());
        return "Receive " + body.toString();
    }

}
