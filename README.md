	Server-----------------------------------Client
	  |	         same proto	    	 |
	  |					 |
	  |-------------  socket  --------->  parent
	  |		   TCP(data)		  ---------> child
	  |				             fork		(exec mpeg123)
	  |					     pipe?
	  |UDP(GROUP)
	  |
      	  |-------thread channel
	  |
	  |-------thread list(per 1s)
	  |
	  |-------media-lib
	  |	   1.mp3
	  |	   2.mp3
          |	    ...
	  |	    ...
		   ?.avi()

    which database? or use file system? how to use database in c++?
