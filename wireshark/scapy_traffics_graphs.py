from scapy.all import *
import matplotlib.pyplot as plt
from collections import defaultdict

packets = rdpcap("stas.pcapng")

time_series = []
tcp_count = []
udp_count = []
dns_count = []
http_count = []

start_time = packets[0].time
interval = 1   # 1 second bins

bins = defaultdict(lambda: {"total":0, "tcp":0, "udp":0, "dns":0, "http":0})

for pkt in packets:
    t = int(pkt.time - start_time)

    bins[t]["total"] += 1

    if pkt.haslayer(TCP):
        bins[t]["tcp"] += 1

        if pkt.haslayer(Raw):
            bins[t]["http"] += 1

    if pkt.haslayer(UDP):
        bins[t]["udp"] += 1

    if pkt.haslayer(DNS):
        bins[t]["dns"] += 1

times = sorted(bins.keys())

for t in times:
    time_series.append(t)
    tcp_count.append(bins[t]["tcp"])
    udp_count.append(bins[t]["udp"])
    dns_count.append(bins[t]["dns"])
    http_count.append(bins[t]["http"])

# Total packets
plt.figure()
plt.plot(time_series, [bins[t]["total"] for t in times])
plt.title("Total Packets vs Time")
plt.xlabel("Time")
plt.ylabel("Packets")
plt.show()

# TCP graph
plt.figure()
plt.plot(time_series, tcp_count)
plt.title("TCP Traffic")
plt.show()

# UDP graph
plt.figure()
plt.plot(time_series, udp_count)
plt.title("UDP Traffic")
plt.show()

# DNS graph
plt.figure()
plt.plot(time_series, dns_count)
plt.title("DNS Traffic")
plt.show()

# HTTP graph
plt.figure()
plt.plot(time_series, http_count)
plt.title("HTTP Traffic")
plt.show()
