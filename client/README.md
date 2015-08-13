Let Client1 have client-id "920dfd80-2eef-11e5-b031-34689524378f" (according to InstaMsg-Server conventions).
Let Client2 have client-id "b5fb75e0-2ef1-11e5-86dd-34689524378f" (according to InstaMsg-Server conventions).


1)
Add Client1 and Client2 in InstaMsg eco-system, using the InstaMsg-Server-Webapp.
Add a topic, say "listener_topic" as a sub_topic for Client1, and as a pub_topic for Client2.

Also, add appropriate passwords.

2)
Compile the binary "stdoutsub", using 

    ./build_instamsg.sh ./Makefile_Linux_Desktop

3)
In one terminal, fire the following for Client1

    build/linux_desktop/instamsg listener_topic --qos 2 --clientid 920dfd80-2eef-11e5-b031-34689524378f --password ajaygarg456 --log /home/ajay/subscriber --sub

This should subscribe Client1 for topic "listener_topic" at the InstaMsg-Server, and must be confirmed from the console-logs of the InstaMsg-Server.


4)
In second terminal (while the first terminal is still active), fire the following for Client2

    build/linux_desktop/instamsg listener_topic --qos 2 --clientid b5fb75e0-2ef1-11e5-86dd-34689524378f --password ajaygarg789 --log /home/ajay/publisher --pub --msg "Test"

This should publish the message on the topic "listener_topic" at the InstaMsg-Server (can be confirmed from the console-logs of the InstaMsg-Server).
But most importantly, the message "this really works !!" must be received properly at Client1 in the first-terminal.
