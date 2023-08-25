from functools import cmp_to_key

tweets = ["Happy #IlliniFriday!",
  "It is a pretty campus, isn't it, #illini?",
  "Diving into the last weekend of winter break like... #ILLINI #JoinTheFight",
  "Are you wearing your Orange and Blue today, #Illini Nation?"]

hashtags = {}

for tweet in tweets:
    for word in tweet.split():
        word = word.lower()
        if word[0] == "#":
            if word not in hashtags:
                hashtags[word] = 0
            hashtags[word] += 1

def sort_fn(a, b):
    if a[1] == b[1]:
	if a[0] > b[0]:
		return 1
	return -1
    return b[1] - a[1]

hashtag_counts = sorted(list(hashtags.items()), key=cmp_to_key(sort_fn))
print(hashtag_counts)
