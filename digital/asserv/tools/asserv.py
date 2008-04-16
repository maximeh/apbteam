import proto, time
import numpy

class Asserv:

    stats_format = {
	    'C': 'HHH',
	    'Z': 'H',
	    'S': 'bbb',
	    'P': 'hhhhhh',
	    'W': 'hhh',
	    }
    # The last occuring stats will increment stats_count, so they have to
    # be in the same order than in asserv program.
    stats_order = 'CZSPW'
    stats_items = {
	    'lc': ('C', 0),
	    'rc': ('C', 1),
	    'a0c': ('C', 2),
	    'a0z': ('Z', 0),
	    'ts': ('S', 0),
	    'as': ('S', 1),
	    'a0s': ('S', 2),
	    'te': ('P', 0),
	    'ti': ('P', 1),
	    'ae': ('P', 2),
	    'ai': ('P', 3),
	    'a0e': ('P', 4),
	    'a0i': ('P', 5),
	    'lw': ('W', 0),
	    'rw': ('W', 1),
	    'a0w': ('W', 2),
	    }

    def __init__ (self, file, **param):
	self.proto = proto.Proto (file, time.time, 0.1)
	def make_handle (s):
	    return lambda *args: self.handle_stats (s, *args)
	for (s, f) in self.stats_format.iteritems ():
	    self.proto.register (s, f, make_handle (s))
	self.stats_enabled = None
	self.param = dict (
		tkp = 0, tki = 0, tkd = 0,
		akp = 0, aki = 0, akd = 0,
		a0kp = 0, a0ki = 0, a0kd = 0,
		E = 1023, I = 1023, b = 15000,
		ta = 0, aa = 0, a0a = 0,
		tsm = 0, asm = 0, tss = 0, ass = 0, a0sm = 0, a0ss = 0
		)
	self.param.update (param)
	self.send_param ()

    def stats (self, *stats_items, **options):
	"""Activate stats."""
	interval = 1
	if 'interval' in options:
	    interval = options['interval']
	# Build list of stats letters.
	stats = [self.stats_items[i][0] for i in stats_items]
	stats = [s for s in self.stats_order if s in stats]
	stats_last_pos = 0
	stats_pos = { }
	for s in stats:
	    stats_pos[s] = stats_last_pos
	    stats_last_pos += len (self.stats_format[s])
	# Build stats item positions.
	self.stats_items_pos = [ ]
	for i in stats_items:
	    id = self.stats_items[i]
	    self.stats_items_pos.append (stats_pos[id[0]] + id[1])
	# Enable stats.
	for s in stats:
	    self.proto.send (s, 'B', interval)
	# Prepare aquisition.
	self.stats_enabled = stats
	self.stats_counter = stats[-1]
	self.stats_count = 0
	self.stats_list = [ ]
	self.stats_line = [ ]

    def get_stats (self, wait = None):
	if wait:
	    self.wait (wait)
	list = self.stats_list
	# Drop first line as it might be garbage.
	del list[0]
	for s in reversed (self.stats_enabled):
	    self.proto.send (s, 'B', 0)
	# Extract asked stats.
	array = numpy.array (list)
	array = array[:, self.stats_items_pos]
	# Cleanup.
	self.stats_enabled = None
	del self.stats_items_pos
	del self.stats_counter
	del self.stats_count
	del self.stats_list
	del self.stats_line
	return array

    def consign (self, w, c):
	"""Consign offset."""
	if w == 't':
	    self.proto.send ('c', 'hh', c, 0)
	elif w == 'a':
	    self.proto.send ('c', 'hh', 0, c)
	else:
	    assert w == 'a0'
	    self.proto.send ('c', 'h', c)

    def send_param (self):
	p = self.param
	self.proto.send ('p', 'BHH', ord ('p'), p['tkp'] * 256,
		p['akp'] * 256)
	self.proto.send ('p', 'BHH', ord ('i'), p['tki'] * 256,
		p['aki'] * 256)
	self.proto.send ('p', 'BHH', ord ('d'), p['tkd'] * 256,
		p['akd'] * 256)
	self.proto.send ('p', 'BH', ord ('p'), p['a0kp'] * 256)
	self.proto.send ('p', 'BH', ord ('i'), p['a0ki'] * 256)
	self.proto.send ('p', 'BH', ord ('d'), p['a0kd'] * 256)
	self.proto.send ('p', 'BH', ord ('E'), p['E'])
	self.proto.send ('p', 'BH', ord ('I'), p['I'])
	self.proto.send ('p', 'BH', ord ('b'), p['b'])
	self.proto.send ('p', 'BHH', ord ('a'), p['ta'] * 256,
		p['aa'] * 256)
	self.proto.send ('p', 'BH', ord ('a'), p['a0a'] * 256)
	self.proto.send ('p', 'BBBBB', ord ('s'), p['tsm'], p['asm'],
		p['tss'], p['ass'])

    def handle_stats (self, stat, *args):
	if self.stats_enabled is not None:
	    self.stats_line.extend (args)
	    if self.stats_counter == stat:
		self.stats_list.append (self.stats_line)
		self.stats_line = [ ]
		self.stats_count += 1

    def wait (self, cond = None):
	try:
	    cond_count = int (cond)
	    cond = lambda: self.stats_count > cond_count
	except TypeError:
	    pass
	self.proto.wait (cond)

    def reset (self):
	self.proto.send ('w')
	self.proto.send ('z')

    def close (self):
	self.reset ()
	self.wait (lambda: True)
	self.proto.file.close ()
