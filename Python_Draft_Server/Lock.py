class Lock:

  def __init__(self, addr, conn, device_code, keypad, my_uid):
    self.ip_addr = addr
    self.conn = conn
    self.device_code = device_code
    self.keypad_uid = keypad
    self.my_uid = my_uid