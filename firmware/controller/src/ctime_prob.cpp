#include "ctime_prob.h"

#include <assert.h>

#include <string>
#include <vector>

struct ctime_prob {
	ctime_prob(const char *name, size_t N)
	    : d_name{name} {
		assert(N > 0);
		assert((N & (N - 1)) == 0);
		d_data.resize(N);
	}

	void push(int64_t value) {
		d_data[d_i++] = value;
		d_i &= d_data.size() - 1;
		if (d_i != 0) {
			return;
		}
		int64_t min{1LL << 62}, max{0}, max2{0}, mean{0};
		for (int64_t v : d_data) {
			min  = std::min(min, v);
			max2 = std::max(max2, v);
			if (v > max) {
				max2 = max;
				max  = v;
			}
			mean += v;
		}
		mean /= d_data.size();
		printf(
		    "ctime %s[%d] mean=%lld.%03lld min=%lld.%03lld max=%lld.%03lld "
		    "max-1=%lld.%03lld\n",
		    d_name,
		    d_data.size(),
		    mean / 1000,
		    mean % 1000,
		    min / 1000,
		    min % 1000,
		    max / 1000,
		    max % 1000,
		    max2 / 1000,
		    max2 % 1000
		);
	}

private:
	std::vector<int64_t> d_data;
	size_t               d_i = 0;
	const char          *d_name;
};

ctime_prob_t *ctime_prob_init(size_t windows_size, const char *name) {
	return new ctime_prob(name, windows_size);
}

void ctime_prob_free(ctime_prob_t *ctime) {
	delete ctime;
}

void ctime_prob_push(ctime_prob_t *ctime, int64_t duration) {
	ctime->push(duration);
}
