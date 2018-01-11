class client_data():
    def __init__(self, ip, num, ind, str):
        self.ip = ip
        self.num = num
        self.ind = ind
        self.units = str.replace("0", "").split("|")
        self.units = filter(None, self.units)
        self.vals = []
        for u in self.units:
          self.vals.append(0)

    def update_client(self, data):
        if not data[0].isdigit:
            raise
        values = data.split("|")
        vals = []
        for val in values:
            #test = float(val)
            if val == "xxx.xxx":
                continue
            #val = val.strip("0")
            while val[0]=='0' and val[1]!='.':
                val = val[1:]
            vals.append(val)
        self.vals = vals

def findClient(clientlist, num):
    for client in clientlist:
        if client.num == num:
            return client
    return None
