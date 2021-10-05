#include "contiki.h"
#include "virtual_sensor.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"
#define LOG_MODULE "Sensor Node App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SEND_INTERVAL (10 * CLOCK_SECOND)

static struct simple_udp_connection connection;

PROCESS(sensor_node, "Sensor node");
AUTOSTART_PROCESSES(&sensor_node);

PROCESS_THREAD(sensor_node, ev, data)
{
    static struct etimer periodic_timer;
    static char buffer[32];
    uip_ipaddr_t server_address;

    PROCESS_BEGIN();

    simple_udp_register(&connection, CLIENT_PORT, NULL, SERVER_PORT, NULL);
    etimer_set(&periodic_timer, SEND_INTERVAL);

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

        uint8_t is_node_reachable = NETSTACK_ROUTING.node_is_reachable();
        uint8_t got_server_address = NETSTACK_ROUTING.get_root_ipaddr(&server_address);

        if(is_node_reachable && got_server_address) {
            LOG_INFO("sending request to");
            LOG_INFO_6ADDR(&server_address);
            LOG_INFO_("\n");

            float temp_reading = read_temperature();
            float hum_reading = read_humidity();

            snprintf(buffer, sizeof(buffer), "temp: %.2f, humidity: %.2f", temp_reading, hum_reading);
            simple_udp_sendto(&connection, buffer, strlen(buffer), &server_address);
        } else {
            LOG_INFO("Root not reachable yet.\n");
        }

        etimer_reset(&periodic_timer);
    }


    PROCESS_END();
}