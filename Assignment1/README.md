# Assignment 1
EE2028 Asssignment 1: Parking System

## key changes from original C code
- result_arr[1] includes [size of entry arr (E), max cars per section (M)], so either of those entries can be dynamic now
- currently performing changes on building_arr directly, result_arr completely untouched (to check if allowed)

## error handling
- if cars entering > available slots, carpark will just fill up to max cars for every section
- if cars leaving > number of cars in that section, that section will just zero out

## assumptions made
- to fill up after questions asked

## questions to ask (collating)
1) can we assume all the input array lengths > 0
2) what are the upper bounds for all the input values?
3) can we edit the values in the building_arr itself and change the C code to print building_arr instead of result_arr
4) can we modify the asm_func to return values instead of just being a void function (e.g. to send error code if overflows occurred)
5) to confirm: in the case of entries overflow, the number of cars will just cut off at SECTION_MAX, then when exiting it will subtract from that value? 
    - e.g. if current cars = 10, entering cars = 3, exiting cars = 1, the processing should be 10+3 --> 12 (cut out extra cars) then 12-1 --> 11
6) can we assume the exiting cars input will never exceed the number of cars in the building section currently
7) are there any preferred things to optimise for e.g. program speed, registers used, modularity (e.g. can vary SECTION_MAX or entry_arr length) or completely up to our choice?

## things left to complete
- [x] entries portion
- [x] exiting portion
- [ ] stress test edge cases
- [ ] optimisation: register usage? program size? speed? reordering instructions (optimise for [pipelining](https://www.cise.ufl.edu/~mssz/CompOrg/CDA-pipe.html#:~:text=or%20relieving%20hazards.-,5.3.3.%20Data%20Hazards,-Definition.%20A), [link2](https://pages.cs.wisc.edu/~fischer/cs701.f14/lectures/L15.pdf))?
- [ ] extra features??

## optimisation tracking
using this test case:
`int building[F][S] = {{9,10},{7,8},{4,4}};
int entry[E] = {2,4,6,8,10};
int exit[F][S] = {{1,1},{1,1},{1,1}};`

for entries portion:
- 141 clock cycles (CC) as of 17/02
- 126 CC by adding entry_arr elements in a loop at the start and rearranging functions at start to reduce dependencies
- 105 CC by assuming E=5, so adding of entry_arr elements can be loop unrolled, optimising for pipelining
- 94 CC by removing subroutine, so no need to push/pop of LR from stack (just be careful not to modify R14)

after adding exiting portion:
- 152 CC, adding exiting as another loop through building_arr and exit_arr
- 146 CC by converting SECTION_OVERFLOW cmp instruction into IT block
- 126 CC by incorporating exiting into entries loop
- 114 CC by removing IT blocks for checking exiting <= current cars in building