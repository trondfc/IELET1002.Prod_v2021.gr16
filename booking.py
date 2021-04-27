import json
import requests
import time

token = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MTA1In0.rtwgSROsIox_OXVB5CLaltp6GdoBp0BA0DtS2Fv3fpM"

badSignal = "12707"
kjokkenSignal = "2384"
stueSignal = "23370"

badTidSignal = "1662"
kjokkenTidSignal = "8185"
stueTidSignal = "15699"

badDusjSignal = "28076"
kjokkenOvnSignal = "13261"

oppvaskSignal = "20610"


bad = {'Key':badSignal,'Value':0,'Token':token}
kjokken = {"Key":kjokkenSignal,"Value":0,"Token":token}
stue = {"Key":stueSignal,"Value":0,"Token":token}

badDusj = {"Key":badDusjSignal,"Value":0,"Token":token} 
kjokkenOvn = {"Key":kjokkenOvnSignal,"Value":0,"Token":token}

badTid = {'Key':badTidSignal,'Value':0,'Token':token}
kjokkenTid = {"Key":kjokkenTidSignal,"Value":0,"Token":token}
stueTid = {"Key":stueTidSignal,"Value":0,"Token":token} 

oppvask = {'Key':oppvaskSignal,'Value':0,'Token':token}



def getValues(rom):
    response =requests.get('https://circusofthings.com/ReadValue',params=rom)
    Value = json.loads(response.content)["Value"]
    return Value

badValue = getValues(bad)
kjokkenValue = getValues(kjokken)
stueValue = getValues(stue)

badTidValue = getValues(badTid)
kjokkenTidValue = getValues(kjokkenTid)
stueTidValue = getValues(stueTid)

badDusjValue = getValues(badDusj)
kjokkenOvnValue = getValues(kjokkenOvn)

oldBadValue = badValue
oldKjokkenValue = kjokkenValue
oldStueValue = stueValue

oppvaskKapasitet = 0

def popReturnList(queue, time, queueDict, timeDict,indexToPop):
    
    queueList = []
    timeList = []

    queue = str(queue)
    time = str(time)
    

    queueList[:0] = queue
    timeList[:0] = time
    
    print(queueList)

    del queueList[indexToPop]
    del timeList[indexToPop]
    

    queueList.append("0")
    timeList.append("0")
    

    queue = ''.join(queueList)
    time = ''.join(timeList)
    

    queueDict["Value"] = int(queue)
    timeDict["Value"] = int(time)
    

    requests.put('https://circusofthings.com/WriteValue',
				data=json.dumps(queueDict),headers={'Content-Type':'application/json'})

    requests.put('https://circusofthings.com/WriteValue',
				data=json.dumps(timeDict),headers={'Content-Type':'application/json'})

    print("success!")
    print(indexToPop)
    return queue, time  
    
def extraChoicePop(ovnswr,ovnswrDict):
    ovnswrList = []
    ovnswr = str(ovnswr)
    ovnswrList[:0] = ovnswr

    del ovnswrList[1]
    ovnswrList.append("0")
    ovnswr = ''.join(ovnswrList)
    ovnswrDict["Value"] = int(ovnswr)

    requests.put('https://circusofthings.com/WriteValue',
				data=json.dumps(ovnswrDict),headers={'Content-Type':'application/json'})
    return ovnswr

def checkQueues(queue1,queue2,queue3): # funksjon som sjekker hvilke signaler det skal telles ned på
    a = False
    b = False
    c = [False,False,False]
    if str(queue1)[1] == "0":
        a = False
    else:
        a = True
    if str(queue2)[1] == "0":
        b = False
    else:
        b = True
    if str(queue3)[1] == "0":
        c = [False,False,False]
    else:
        i = 1
        sum = 0 #sum for antall i stua
        while i <= 3:
           sum += guestList[int(str(queue3)[i])] + 1
           if sum <= 3:
            c[i-1] = True
           i+=1

    return a,b,c

timerBad = time.time()
timerKjokken = time.time()
timerStue1 = time.time()
timerStue2 = time.time()
timerStue3 = time.time()

kjokkenCountDown = False
badCountDown = False
stueCountDownList = [False,False,False] #Hvilke indekser som skal telles ned i stua, maks 3 I stua inkludert besøk, må hente hvilke beboere som har besøk
guestList = [0,1,2,1,0,0] #Liste for hvem som har besøk, indeks er id nummer til beboer og celle verdi er antall beboer har på besøk 


def countDownFunc(countDown,signalValue,timeSignalValue,signalDict,timeDict,indexToPop):
    timeNow = time.time()
    if countDown == True:
        if (timerStue3 + int(str(signalValue)[3])*15*60) < timeNow:
            stueValue, stueTidValue = popReturnList(signalValue,timeSignalValue,signalDict,timeDict,indexToPop)
            timerStue3 = timeNow
    return stueValue,stueTidValue

i = 0
while True:
    if i > 60:
        badValue = getValues(bad)
        kjokkenValue = getValues(kjokken)
        stueValue = getValues(stue)

        badTidValue = getValues(badTid)
        kjokkenTidValue = getValues(kjokkenTid)
        stueTidValue = getValues(stueTid)

        badDusjValue = getValues(badDusj)
        kjokkenOvnValue = getValues(kjokkenOvn)
        
        i = 0

    print(stueCountDownList)
    badCountDown, kjokkenCountDown, stueCountDownList = checkQueues(badValue,kjokkenValue,stueValue)
    timeNow = time.time()

    if badCountDown:
        if (timerBad + int(str(badTidValue)[1])*5*60) < timeNow:
            badValue, badTidValue = popReturnList(badValue,badTidValue,bad,badTid,1)
            badDusjValue = extraChoicePop(badDusjValue,badDusj)
            timerBad = timeNow
    else:
        timerBad = time.time()
    if kjokkenCountDown:
        if (timerKjokken + int(str(kjokkenTidValue)[1])*5*60) < timeNow:
            kjokkenValue, kjokkenTidValue = popReturnList(kjokkenValue,kjokkenTidValue,kjokken,kjokkenTid,1)
            kjokkenOvnValue = extraChoicePop(kjokkenOvnValue,kjokkenOvn)
            oppvaskKapasitet += 10
            if oppvaskKapasitet == 100:
                oppvask["Value"] = 1
                requests.put('https://circusofthings.com/WriteValue',
				    data=json.dumps(oppvask),headers={'Content-Type':'application/json'}) 
                oppvaskKapasitet = 0


            timerKjokken = timeNow
    else:
        timerKjokken = timeNow
    if stueCountDownList[0] == True:
        if (timerStue1 + int(str(stueTidValue)[1])*15*60) < timeNow:
            stueValue, stueTidValue = popReturnList(stueValue,stueTidValue,stue,stueTid,1)
            timerStue2 = timerStue1 + int(str(stueTidValue)[1])*15*60
            timerStue3 = timerStue2 + int(str(stueTidValue)[2])*15*60
            timerStue1 = timeNow
    else:
        timerStue1 = timeNow

    if stueCountDownList[1] == True:
        if (timerStue2 + int(str(stueTidValue)[2])*15*60) < timeNow:
            stueValue, stueTidValue = popReturnList(stueValue,stueTidValue,stue,stueTid,2)
            timerStue3 = timerStue2 + int(str(stueTidValue)[3])*15*60
            timerStue2 = timeNow
    else:
        timerStue2 = timeNow

    if stueCountDownList[2] == True:
        if (timerStue3 + int(str(stueTidValue)[3])*15*60) < timeNow:
            stueValue, stueTidValue = popReturnList(stueValue,stueTidValue,stue,stueTid,3)
            timerStue3 = timeNow
    else:
        timerStue3 = timeNow
    
    time.sleep(1)
    i += 1

    oldBadValue = badValue
    oldKjokkenValue = kjokkenValue
    oldStueValue = stueValue




