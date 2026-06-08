# Alphabet Language — User Research Plan

## Research Questions

1. **Who are our users?**
   - Age, education, programming experience
   - Why do they want to learn programming?
   - What languages do they speak?

2. **What do they need?**
   - Simple syntax? Multilingual support? Voice input?
   - What features matter most?
   - What's missing?

3. **How do they learn?**
   - Self-taught? Classroom? Online courses?
   - What resources do they use?
   - What frustrates them?

4. **What makes them stick?**
   - What keeps them using Alphabet?
   - What makes them switch to another language?
   - What's the "aha" moment?

## Research Methods

### 1. Surveys
- **Target:** 100+ potential users
- **Questions:** 15-20 multiple choice + open ended
- **Distribution:** Reddit, Twitter, Discord, university CS departments
- **Timeline:** 2 weeks

### 2. Interviews
- **Target:** 10-15 users
- **Format:** 30-minute video calls
- **Questions:** Open-ended about learning experience
- **Timeline:** 3 weeks

### 3. Usability Testing
- **Target:** 5-10 new users
- **Format:** Watch them use Alphabet for 30 minutes
- **Tasks:** Complete 5 basic programming tasks
- **Metrics:** Success rate, time, errors, frustration
- **Timeline:** 2 weeks

### 4. Analytics
- **Tools:** Opt-in telemetry (future)
- **Metrics:** Feature usage, error rates, session length
- **Timeline:** Ongoing

## Key Findings (Hypotheses)

### H1: Multilingual is the killer feature
- Non-English speakers struggle with English keywords
- Amharic speakers can't easily type Latin characters
- Voice input removes typing barrier

### H2: Single-letter keywords are intimidating
- Beginners prefer full words (if, else, loop)
- Experts prefer short keywords (i, e, l)
- Solution: Both work (if==i, else==e)

### H3: Type IDs confuse beginners
- `5 x = 42` is unclear
- `integer x = 42` is better
- `let x = 42` is best for beginners
- Solution: All three work

### H4: Voice input is a novelty
- Users try it once, then type
- Useful for Amharic (hard to type)
- Useful for accessibility

## Action Items

- [ ] Create survey (Google Forms)
- [ ] Recruit interview participants
- [ ] Design usability test tasks
- [ ] Analyze results
- [ ] Prioritize features based on research
