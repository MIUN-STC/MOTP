# objtrack
Example on how to track objects

### Strategy
1. **Only one tracker per trail.**<br>
There is big chance a trail will consume many tracker if otherwise. There is limited supply of trackers. 
2. **A tracker has a mass and changes its force direction towards a trail.**<br>
A trail is not necessarily from target the tracker is tracking. The tracker will be less distracted by false trails by keeping its momentum.<br>
<a href="https://www.codecogs.com/eqnedit.php?latex=\LARGE&space;\ddot{x}&space;=&space;d&space;\frac{1}{m}&space;\frac{1}{u}" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\LARGE&space;\ddot{x}&space;=&space;d&space;\frac{1}{m}&space;\frac{1}{u}" title="\LARGE \ddot{x} = d \frac{1}{m} \frac{1}{u}" /></a>
3. **A tracker can only track a trail within a search radius.**<br>
A object that has recently been tracked can not go very far away from the observed position.<br>
<a href="https://www.codecogs.com/eqnedit.php?latex=\LARGE&space;\left&space;|&space;z&space;-&space;x&space;\right&space;|&space;<&space;r" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\LARGE&space;\left&space;|&space;z&space;-&space;x&space;\right&space;|&space;<&space;r" title="\LARGE \left | z - x \right | < r" /></a>
4. **A tracker tracks the most similar trail based on which trail characteristics the tracker is tracking.**<br>
There is allways a chance that multiple trails can exist within a search radius.<br>
The tracker has to pick the most similar trail.<br>
Example of trail characteristics: trail position and trail size.<br>
<a href="https://www.codecogs.com/eqnedit.php?latex=\LARGE&space;d&space;=&space;z&space;-&space;x" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\LARGE&space;d&space;=&space;z&space;-&space;x" title="\LARGE d = z - x" /></a>
5. **A tracker slows down if there is no trail inside the search radius.**<br>
The tracker does not know where the object is going when there is no trail.
The object might change its direction while the tracker keeps going in the direction based on too old information.
6. **A tracker increases its search radius if there is no trail inside the search radius.**<br>
The tracker does not know the position of the target so there is a chance that the target is outside the search radius.<br>
<a href="https://www.codecogs.com/eqnedit.php?latex=\huge&space;r&space;=&space;r_0&space;&plus;&space;r_ku" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\huge&space;r&space;=&space;r_0&space;&plus;&space;r_ku" title="\huge r = r_0 + r_ku" /></a>
7. **A tracker decreses its search radius if there is a trail inside the search radius.**<br>
The tracker knows the position of the target there is no need to search a where it is impossable for the target to appear.


