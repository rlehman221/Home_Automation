class Keypad:

  def __init__(self, addr, conn, lock, my_uid):
    self.ip_addr = addr
    self.conn = conn
    self.lock_uid = lock
    self.my_uid = my_uid