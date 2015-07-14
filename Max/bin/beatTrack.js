inlets = 1;
outlets = 1;

var Q=new Array();
var maxTime = 6500;
var allowedDrift = .03;
var beatoGram  = new Array();
var minBPM = 59;
var maxBPM = 309;
	
function tick(interval){
	if(interval>60000.0*.1/maxBPM){
		Q.unshift(interval);
		cutQ();
		//post("Q",Q,"\n");
		beatogram();
		if(beatoGram.length>4){
		getMainBpm();
}
		//for ( var k in beatoGram){post("b",k,beatoGram[k],"\n");}
			}
}


function cutQ(){
	var t=0;
	var idx = 0;
	for(var i in Q){
		t+=Q[i];
		idx++;
		if(t>maxTime){break;}
	}
	for (var i = idx;i < Q.length ; i++){
Q.pop();

}
}

function beatogram(){
	beatoGram = new Array();
	for(var i in Q){
		
		var interval = 0;
		for (var j = i; j < Q.length;j++){ 
			var key = -1;
			interval +=Q[j] 
			// if (interval > 60000.0/minBPM)break;
 
		for(var k in beatoGram){
			var diff = Math.abs((beatoGram[k].target / interval) - 1);
			
			if (diff < allowedDrift){ 
				// post("dif",diff ,k,"\n")
				key = k; 
				beatoGram[k].num+=1 ;
	 			beatoGram[k].targets.push(interval);
	 			beatoGram[k].target = median(beatoGram[k].targets) 
				break;
			}
		}
		
		if(key == -1 && interval > 60000.0/maxBPM){
			// post ("adding" , Q[i],"\n");
			var beat ={};
			beat.num = 1;
			beat.targets = new Array();
			beat.targets.push(interval);
			beat.target = interval;
			beatoGram.push(beat);
			 }
		}		
	}	

	beatoGram.sort(function(a,b){return(a.target-b.target)})
	// for(k in beatoGram){post ("beats",beatoGram[k].target);}
}

function getMainBpm(){
	harmBeats = beatoGram.slice()


	 for(var b in beatoGram){
	 	for(var bb = b ; bb < beatoGram.length ; bb++){
	 		if(b!=bb){
	 			var closeenough = false;
	 			var comp = beatoGram[bb].target;
				var weight = 1;
				if(Math.abs(comp/beatoGram[b] - 1) < allowedDrift){closeenough = true;}
					else{
	 			while(comp>beatogram[b]){
	 				comp/=2;
	 				weight/=1.5;
	 				if(Math.abs(comp/beatoGram[b] - 1) < allowedDrift){closeenough = true; break;}
	 			}
	}
	 			if(closeenough){harmBeats[b].num+=weight;}
		
	 		}
	 	}
	 }
	for(h in harmBeats){
		 harmBeats[h].num *=weightBPM(60000.0/harmBeats[h].target)
	}
	var fh = 0;
	var maxScore = 0;
	for(h in harmBeats){

		if(harmBeats[h].num >maxScore){
			maxScore=harmBeats[h].num;
			fh = h;
		}
	}
	
	bpm = mean(harmBeats[fh].targets);



	// post(idx,"\n")
	


	// harmBeats.sort(function(a,b){return(b.num-a.num)})
	// for(k in harmBeats){post("h",harmBeats[k].target,harmBeats[k].num)}
	// post("\n");

	outlet(0,60000.0/bpm);

}

function median(_arr){
arr = _arr.slice()
		arr.sort();
	var idx = arr.length/2;
	if(idx%1==0){
		return arr[idx];
	}
	else{
		idx-=idx%1;
		if(arr.length>1){
		return (arr[idx] + arr[idx+1])  /2 ;
		}
		else{
		return arr[0];
		}
	}
}

function mean(arr){
	 			var sum = 0;
	 			for(var i in arr){
	 				sum+=arr[i];

	 			}
	 			return sum*1.0/(arr.length);
}

function weightBPM(b){
	var base = 0.4;
	var spread = maxBPM-minBPM;
	return (1-base)*Math.cos(Math.PI*(b-spread/2-minBPM)/spread) + base
	
}