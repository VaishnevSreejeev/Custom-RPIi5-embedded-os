#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

#define MAX_BYTES 2048

// Helper to sanitize and print payload data (similar to Python's decode errors='ignore')
void print_payload(const u_char *payload, int len) {
    printf(" | Data: ");
    int print_len = len > 50 ? 50 : len; // Limit to 50 chars like your Python script
    for (int i = 0; i < print_len; i++) {
        if (isprint(payload[i])) {
            printf("%c", payload[i]);
        } else {
            printf(".");
        }
    }
    if (len > 50) printf("...");
}

void packet_handler(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    struct ether_header *eth_header;
    struct ip *ip_header;
    
    // 1. Parse Ethernet Header
    eth_header = (struct ether_header *)packet;
    if (ntohs(eth_header->ether_type) != ETHERTYPE_IP) {
        return; // Only process IP packets
    }

    // 2. Parse IP Header
    // The IP header follows the Ethernet header (size is usually 14 bytes)
    ip_header = (struct ip *)(packet + sizeof(struct ether_header));
    
    char src_ip[INET_ADDRSTRLEN];
    char dst_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ip_header->ip_src), src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip_header->ip_dst), dst_ip, INET_ADDRSTRLEN);

    printf("[%s] -> [%s]", src_ip, dst_ip);

    // Calculate offset to Transport Layer
    // ip_hl is length in 32-bit words, so multiply by 4 to get bytes
    int ip_header_len = ip_header->ip_hl * 4;
    
    // Pointer to Protocol Header (TCP/UDP/ICMP)
    const u_char *proto_packet = packet + sizeof(struct ether_header) + ip_header_len;
    int total_header_len = sizeof(struct ether_header) + ip_header_len;
    int payload_len = pkthdr->len - total_header_len;

    // 3. Transport Layer Analysis
    if (ip_header->ip_p == IPPROTO_TCP) {
        struct tcphdr *tcp = (struct tcphdr *)proto_packet;
        int sport = ntohs(tcp->source);
        int dport = ntohs(tcp->dest);
        
        printf(" | TCP %d->%d", sport, dport);

        // TCP Header length
        int tcp_header_len = tcp->doff * 4;
        const u_char *payload = proto_packet + tcp_header_len;
        int data_len = payload_len - tcp_header_len;

        // HTTP Check (Port 80)
        if (sport == 80 || dport == 80) {
            printf(" [HTTP]");
        }
        // HTTPS Check (Port 443)
        else if (sport == 443 || dport == 443) {
            printf(" [HTTPS/TLS] (Encrypted)");
        }

        if (data_len > 0) print_payload(payload, data_len);

    } else if (ip_header->ip_p == IPPROTO_UDP) {
        struct udphdr *udp = (struct udphdr *)proto_packet;
        int sport = ntohs(udp->source);
        int dport = ntohs(udp->dest);

        printf(" | UDP %d->%d", sport, dport);

        // UDP Header is fixed 8 bytes
        const u_char *payload = proto_packet + sizeof(struct udphdr);
        int data_len = payload_len - sizeof(struct udphdr);

        // DNS Check (Port 53) - Basic check
        if (sport == 53 || dport == 53) {
            printf(" [DNS Activity]");
        }
        
        if (data_len > 0) print_payload(payload, data_len);

    } else if (ip_header->ip_p == IPPROTO_ICMP) {
        struct icmp *icmp_header = (struct icmp *)proto_packet;
        printf(" | ICMP Type: %d", icmp_header->icmp_type);
    }

    printf("\n");
}

int main(int argc, char *argv[]) {
    char *dev;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    // Interface selection: default to wlan0 (RPi standard) if not provided
    if (argc > 1) {
        dev = argv[1];
    } else {
        dev = "wlan0";
    }

    printf("[*] Sniffing on %s... (Press Ctrl+C to stop)\n", dev);

    // Open session in promiscuous mode
    // arguments: device, snapshot len, promiscuous, timeout ms, error buffer
    handle = pcap_open_live(dev, MAX_BYTES, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return(2);
    }

    // Check if we are capturing Ethernet headers (DLT_EN10MB)
    if (pcap_datalink(handle) != DLT_EN10MB) {
        fprintf(stderr, "Device %s doesn't provide Ethernet headers - not supported\n", dev);
        return(2);
    }

    // Start Loop
    pcap_loop(handle, -1, packet_handler, NULL);

    pcap_close(handle);
    return(0);
}