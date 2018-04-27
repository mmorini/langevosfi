"""
This file contains classes that implement logic of different early termination conditions.
"""

class EarlyTerminator(object):
	# Base class
	pass

class ComprehensionPlateauTerminator(EarlyTerminator):
	"""
	This early terminator ends the run if the following conditions are satisfied:

	1) [current comprehension] >= 
	      min_comprehension_mult * [comprehension at very first reporting interval]
	2) [current interval] >= plateau_time *
	      ([first reporting interval at which comprehension was >=
	        plateau_percentage * [current comprehension])
	"""
	def __init__(self, min_comprehension_mult, plateau_percentage, plateau_time):
		assert(min_comprehension_mult is not None)
		assert(plateau_percentage is not None)
		assert(plateau_time is not None)
		self.min_comprehension_mult = float(min_comprehension_mult)
		self.plateau_percentage = float(plateau_percentage)
		self.plateau_time = float(plateau_time)

	def __repr__(self):
		return self.__class__.__name__ + \
		       "(min_comprehension_mult=%s,plateau_percentage=%s,plateau_time=%s)" \
		       % (self.min_comprehension_mult, self.plateau_percentage, self.plateau_time)

	def terminateCondition(self, stats, grammars):
		cur_comprehension = stats[-1]['Comprehension']
		min_comprehension = self.min_comprehension_mult * stats[0]['Comprehension']
		if cur_comprehension <= min_comprehension:
			return False

		first_interval = None
		for report_interval, interval_stats in enumerate(stats):
			if interval_stats['Comprehension'] >= self.plateau_percentage * cur_comprehension:
				first_interval = report_interval
				break

		if first_interval is None:
			return False
		
		if float(len(stats))/first_interval < self.plateau_time:
			return False

		return True



VALID_TERMINATOR_CLASSES = [ComprehensionPlateauTerminator,]