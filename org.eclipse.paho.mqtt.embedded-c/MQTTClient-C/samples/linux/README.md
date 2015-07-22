Let Client1 have client-id "920dfd80-2eef-11e5-b031-34689524378f" (according to InstaMsg-Server conventions).
Let Client2 have client-id "b5fb75e0-2ef1-11e5-86dd-34689524378f" (according to InstaMsg-Server conventions).


1)
Add Client1 and Client2 in InstaMsg eco-system, using the InstaMsg-Server-Webapp.
Add a topic, say "listener_topic" as a sub_topic for Client1, and as a "pub_topic" for Client2.

Also, add appropriate passwords.

2)
Compile the binary "stdoutsub", using "sh build.sh" OR "sh compile.sh"

3)
In one terminal, fire

	./stdoutsub "listener_topic" --clientid "920dfd80-2eef-11e5-b031" --username 34689524378f  --password ajaygarg456 --qos 0 --sub

(NOTE THE SPLITTING of InstaMsg-Server-ClientId AS THE OPTIONS).

This should subscribe Client1 for topic "listener_topic" at the InstaMsg-Server, and must be confirmed from the console-logs of the InstaMsg-Server.


4)
In second terminal (while the first terminal is still active), fire

	./stdoutsub "listener_topic" --clientid "b5fb75e0-2ef1-11e5-86dd" --username 34689524378f  --password ajaygarg789 --qos 0 --pub --msg "this really works !!"

(AGAIN, NOTE THE SPLITTING of InstaMsg-Server-ClientId AS THE OPTIONS).

This should publish the message on the topic "listener_topic" at the InstaMsg-Server (can be confirmed from the console-logs of the InstaMsg-Server).
But most importantly, the message "this really works !!" must be received properly at Client1 in the first-terminal.
