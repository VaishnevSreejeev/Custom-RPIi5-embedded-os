import sys
from scapy.all import sniff, IP, TCP, UDP, ICMP, DNS, DNSQR, Raw, load_layer

# Load HTTP layer for deep inspection
load_layer("http")

def process_packet(packet):
    # Only process IP packets
    if not packet.haslayer(IP):
        return

    ip = packet[IP]
    proto_info = f"[{ip.src}] -> [{ip.dst}]"
    app_info = ""

    # Transport Layer Analysis
    if packet.haslayer(TCP):
        sport, dport = packet[TCP].sport, packet[TCP].dport
        proto_info += f" | TCP {sport}->{dport}"
        
        # Application Layer Guesses based on Port/Content
        if dport == 80 or sport == 80:
            app_info = "[HTTP]"
            if packet.haslayer("HTTPRequest"):
                req = packet["HTTPRequest"]
                app_info += f" Method: {req.Method.decode()} Host: {req.Host.decode()}"
        elif dport == 443 or sport == 443:
             app_info = "[HTTPS/TLS] (Encrypted)"

    elif packet.haslayer(UDP):
        sport, dport = packet[UDP].sport, packet[UDP].dport
        proto_info += f" | UDP {sport}->{dport}"
        
        # DNS Inspection
        if packet.haslayer(DNS) and packet.haslayer(DNSQR):
            query = packet[DNSQR].qname.decode()
            app_info = f"[DNS] Query: {query}"

    elif packet.haslayer(ICMP):
        proto_info += f" | ICMP Type: {packet[ICMP].type}"

    # Raw Payload Inspection (Deep Dive)
    if packet.haslayer(Raw):
        payload = packet[Raw].load
        # Try to identify text-based protocols (like MQTT, FTP, etc)
        try:
            readable_payload = payload.decode('utf-8', errors='ignore')
            clean_payload = readable_payload.strip().replace('\n', ' ')[:50] # First 50 chars
            if clean_payload:
                app_info += f" | Data: {clean_payload}..."
        except:
            pass

    # Final Log
    print(f"{proto_info} {app_info}")

# Interface Handling
# NOTE: 'wlp0s20f3' is for your Laptop. Change to 'wlan0' when running on Raspberry Pi!
iface = sys.argv[1] if len(sys.argv) > 1 else "wlp0s20f3" 
print(f"[*] Sniffing on {iface}... (Press Ctrl+C to stop)")
sniff(iface=iface, prn=process_packet, store=False)