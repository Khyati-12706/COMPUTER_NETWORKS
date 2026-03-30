import pandas as pd

# ---------- FUNCTION TO LOAD WIRESHARK CSV ----------
def load_conv(file):

    df = pd.read_csv(file, header=None)
    df = df[0].str.split(",", expand=True)

    df.columns = df.iloc[0]
    df = df[1:]

    # clean addresses
    df["Address A"] = df["Address A"].astype(str).str.replace('"','').str.strip()
    df["Address B"] = df["Address B"].astype(str).str.replace('"','').str.strip()

    # clean packets
    df["Packets"] = df["Packets"].astype(str).str.replace('"','').str.strip()
    df["Packets"] = pd.to_numeric(df["Packets"], errors="coerce").fillna(0)

    # clean bytes
    def convert_bytes(x):
        if pd.isna(x):
            return 0
        x = str(x).replace('"','').strip()
        if "kB" in x:
            return float(x.replace("kB","").strip()) * 1000
        elif "bytes" in x:
            return float(x.replace("bytes","").strip())
        else:
            return 0

    df["Bytes"] = df["Bytes"].apply(convert_bytes)

    return df


# ---------- LOAD FILES ----------
ip_df = load_conv("ip_conv.csv")
tcp_df = load_conv("tcp_conv.csv")
udp_df = load_conv("udp_conv.csv")

# ---------- 1. PAIR WITH MAXIMUM DATA ----------
max_row = ip_df.loc[ip_df["Bytes"].idxmax()]

print("\n========= Pair with Maximum Data Transfer =========")
print(max_row["Address A"], "<->", max_row["Address B"])
print("Total Bytes:", int(max_row["Bytes"]))


# ---------- 2. TOTAL PACKETS BETWEEN EVERY PAIR ----------
print("\n========= Packets Between Every Pair =========")

for _, row in ip_df.iterrows():
    print(row["Address A"], "<->", row["Address B"], ":", int(row["Packets"]))


# ---------- 3. AVERAGE INTER-PACKET TIME ----------
# using Duration / Packets approximation from Wireshark conv table

print("\n========= Average Inter-Packet Time =========")

ip_df["Duration"] = ip_df["Duration"].astype(str).str.replace('"','').str.strip()
ip_df["Duration"] = pd.to_numeric(ip_df["Duration"], errors="coerce").fillna(0)

for _, row in ip_df.iterrows():

    packets = row["Packets"]
    duration = row["Duration"]

    if packets > 1:
        avg_time = duration / packets
        print(row["Address A"], "<->", row["Address B"], ":", round(avg_time,4), "sec")
    else:
        print(row["Address A"], "<->", row["Address B"], ": Not enough packets")


# ---------- 4. PROTOCOL WISE PACKET COUNT ----------
print("\n========= Protocol-wise Packet Count =========")

print("Total IP packets  :", int(ip_df["Packets"].sum()))
print("Total TCP packets :", int(tcp_df["Packets"].sum()))
print("Total UDP packets :", int(udp_df["Packets"].sum()))
