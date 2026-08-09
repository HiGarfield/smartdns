#!/bin/bash
# Script to analyze and categorize commits

cd /home/runner/work/smartdns/smartdns

# Get all commits between current HEAD and upstream/master
git log --reverse --format="%H|%s|%an|%ad" --date=short --no-merges 3a06a4d..upstream/master > /tmp/commit_list.txt

echo "=== COMMIT ANALYSIS REPORT ==="
echo "Total commits to analyze: $(wc -l < /tmp/commit_list.txt)"
echo ""

echo "=== POTENTIAL FIX COMMITS (grep 'fix\|Fix\|crash\|leak\|bug\|Bug') ==="
grep -iE 'fix|crash|leak|bug' /tmp/commit_list.txt | cut -d'|' -f1,2 | head -150

echo ""
echo "=== FEATURE COMMITS (grep 'feat\|feature\|add\|Add\|support') ==="
grep -iE 'feat|feature|add.*support|support.*for' /tmp/commit_list.txt | cut -d'|' -f1,2 | head -50

echo ""
echo "=== REFACTOR COMMITS (grep 'refactor') ==="
grep -iE 'refactor|restructure|reorganize' /tmp/commit_list.txt | cut -d'|' -f1,2 | head -30
