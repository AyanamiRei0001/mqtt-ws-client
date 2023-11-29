#include <string>
// mongoose
#include "../lib/mongoose/mongoose.h"

static const char *s_url =
#if MG_TLS
    "wss://broker.hivemq.com:8884/mqtt";
#else
    "ws://localhost:8083/mqtt";
#endif

static const char *s_topic = "mg/test";

static bool s_connected = false;
struct mg_mqtt_opts s_opts;
int count = 0;

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_ERROR)
    {
        // On error, log error message
        MG_ERROR(("%p %s", c->fd, (char *)ev_data));
    }
    // // TODO: ssl
    // else if (ev == MG_EV_CONNECT)
    // {
    //     if (mg_url_is_ssl(s_url))
    //     {
    //         struct mg_tls_opts opts = {.ca = mg_unpacked("/certs/ca.pem"),
    //                                    .name = mg_url_host(s_url)};
    //         mg_tls_init(c, &opts);
    //     }
    // }
    else if (ev == MG_EV_WS_OPEN)
    {
        // WS connection established. Perform MQTT login
        MG_INFO(("Connected to WS. Logging in to MQTT..."));
        // TODO: mqtt connect client_id
        // s_opts.client_id = mg_str("test");
        // TODO: with username and password
        // m_opts.user = mg_str(username.c_str());
        // m_opts.pass = mg_str(password.c_str());

        // struct mg_mqtt_opts opts = {};
        // opts.qos = 1, opts.topic = mg_str(s_topic), opts.message = mg_str("goodbye");
        size_t len = c->send.len;
        mg_mqtt_login(c, &s_opts);
        mg_ws_wrap(c, c->send.len - len, WEBSOCKET_OP_BINARY);
        // c->is_hexdumping = 1;
        s_connected = true;
    }
    if (s_connected && ev == MG_EV_POLL && !c->is_writable)
    {
        // TODO: callback here when we are connected
        std::string message = "hello world! count: " + std::to_string(++count);
        struct mg_str topic = mg_str(s_topic), data = mg_str(message.c_str());
        // get current len like MG_EV_WS_OPEN
        size_t len = c->send.len;

        struct mg_mqtt_opts pub_opts;
        memset(&pub_opts, 0, sizeof(pub_opts));
        pub_opts.topic = topic; // mqtt topic name
        pub_opts.message = data; // mqtt message
        pub_opts.qos = 0, // mqtt topic qos
        pub_opts.retain = false;
        // publish message
        mg_mqtt_pub(c, &pub_opts);
        MG_INFO(("PUBLISHED %.*s -> %.*s", (int)data.len, data.ptr,
                 (int)topic.len, topic.ptr));
        // do this like MG_EV_WS_OPEN
        len = mg_ws_wrap(c, c->send.len - len, WEBSOCKET_OP_BINARY);
    }
    if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE)
    {
        s_connected = false;
        *(bool *)fn_data = true; // Signal that we're done
        MG_ERROR(("MG_EV_ERROR or MG_EV_CLOSE."));
    }
}

int main(void)
{
    struct mg_mgr mgr;                                 // Event manager
    bool done = false;                                 // Event handler flips it to true when done
    mg_mgr_init(&mgr);                                 // Initialise event manager
    mg_log_set(MG_LL_DEBUG);                           // Set log level
    // TODO: (important to use protocol, if not use and cannot send message!)
    mg_ws_connect(&mgr, s_url, fn, &done, "%s",        // Create client connection
                  "Sec-Websocket-Protocol: mqtt\r\n"); // Request MQTT protocol 
    while (done == false)
        mg_mgr_poll(&mgr, 1000); // Event loop (1 times/s)
    mg_mgr_free(&mgr);           // Finished, cleanup
    return 0;
}
