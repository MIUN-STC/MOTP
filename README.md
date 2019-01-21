# objtrack
Example on how to track objects

### Strategy
1. **Only one tracker per trail.**<br>
There is big chance a trail will consume many tracker if otherwise. There is limited supply of trackers. 
2. **A tracker has a mass and changes its force direction towards a trail.**<br>
A trail is not necessarily the target the tracker is tracking. The tracker will be less distracted by false trails by keeping its momentum.
3. **A tracker can only track a trail within a search radius.**<br>
A object that has recently been tracked can not go very far away from the observed position.
4. **A tracker tracks the most similar trail based on which trail characteristics the tracker is tracking.**<br>
There is allways a chance that multiple trails can exist within a search radius.<br>
The tracker has to pick the most similar trail.<br>
Example of trail characteristics: trail position and trail size.
5. **A tracker slows down if there is no trail inside the search radius.**<br>
The tracker does not know where the object is going when there is no trail.
The object might change its direction while the tracker keeps going in the direction based on too old information.
6. **A tracker increases its search radius if there is no trail inside the search radius.**<br>
The tracker does not know the position of the target so there is a chance that the target is outside the search radius.
7. **A tracker decreses its search radius if there is a trail inside the search radius.**<br>
The tracker knows the position of the target there is no need to search a where it is impossable for the target to appear.
