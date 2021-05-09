import json
import requests
import time

from cotLib import COT

token = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MTA1In0.rtwgSROsIox_OXVB5CLaltp6GdoBp0BA0DtS2Fv3fpM"

badSignal = "12707" #Signaler
kjokkenSignal = "2384"
stueSignal = "23370"

badTidSignal = "1662" #hvert siffer representerer antal 5-min
kjokkenTidSignal = "8185" #hvert siffer representerer antal 5-min
stueTidSignal = "15699"#hvert siffer representerer antal 15-min

badDusjSignal = "28076" #om personen skal dusje eller ikke
kjokkenOvnSignal = "13261" # om personen skal bruke ovnen eller ikke

oppvaskSignal = "20610"
gjesteSignal = "4747" #gjester, indeks = beboer id, indeksverdi = antall gjester den beboeren har

bad = {'Key':badSignal,'Value':0,'Token':token} # dictionaries for signaler
kjokken = {"Key":kjokkenSignal,"Value":0,"Token":token}
stue = {"Key":stueSignal,"Value":0,"Token":token}

badDusj = {"Key":badDusjSignal,"Value":0,"Token":token} 
kjokkenOvn = {"Key":kjokkenOvnSignal,"Value":0,"Token":token}

badTid = {'Key':badTidSignal,'Value':0,'Token':token}
kjokkenTid = {"Key":kjokkenTidSignal,"Value":0,"Token":token}
stueTid = {"Key":stueTidSignal,"Value":0,"Token":token} 

oppvask = {'Key':oppvaskSignal,'Value':0,'Token':token}
gjester = {'Key':gjesteSignal,'Value':0,'Token':token}



def getValues(rom): #Funksjon som henter verdier fra CoT
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
gjesteValue = getValues(gjester)

guestList = COT.to_list(1,gjesteValue,[1,1,1,1,1,1,1]) #Liste for hvem som har besøk, indeks er id nummer til beboer og celle verdi er antall beboer har på besøk 
del guestList[0] #fjerner første ener får å gjøre den lettere å jobbe med

#guestList = [1,0,2,0,0,0] 


oppvaskKapasitet = 0

def popReturnList(queue, time, queueDict, timeDict,indexToPop): #fjerner et valgt element fra listene og returnerer listene
    
    queueList = []
    timeList = []

    queue = str(queue)
    time = str(time)
    

    queueList[:0] = queue #gjør om til en liste
    timeList[:0] = time
    

    del queueList[indexToPop] #sletter riktig indeks
    del timeList[indexToPop]
    

    queueList.append("0") #legger på en null slik at listen har samme lengde
    timeList.append("0")
    

    queue = ''.join(queueList) #gjør om til en string
    time = ''.join(timeList)
    

    queueDict["Value"] = int(queue)
    timeDict["Value"] = int(time)
    

    requests.put('https://circusofthings.com/WriteValue',
				data=json.dumps(queueDict),headers={'Content-Type':'application/json'})

    requests.put('https://circusofthings.com/WriteValue',
				data=json.dumps(timeDict),headers={'Content-Type':'application/json'})

    print("success!")
    print(indexToPop)
    print(queueDict["Key"])
    return queue, time  
    
def extraChoicePop(ovnswr,ovnswrDict): # Funksjon for å fjerne element i ekstra valg listen
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
    if str(queue1)[1] == "0": #sjekker om det er noen i køen
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
        i = 1 # i begynner på 1 fordi listene har en ekstra 1 på starten av tallet
        sum = 0 #sum for antall i stua
        while i < len(str(queue3).split('0')[0]): #lengden av queue 3 som ikke har null
            #print(len(str(queue3).split('0')[0]))
            sum += int(guestList[int(str(queue3)[i])-1]) + 1 #legger til personen som har booka, pluss eventuelle gjester
            print(sum)
            if sum <= 3:
                c[i-1] = True
            i+=1
        #print(sum)

    return a,b,c

timerBad = time.time()
timerKjokken = time.time()
timerStue1 = time.time()
timerStue2 = time.time()
timerStue3 = time.time()

kjokkenCountDown = False
badCountDown = False
stueCountDownList = [False,False,False] #Hvilke indekser som skal telles ned i stua, maks 3 I stua inkludert besøk, må hente hvilke beboere som har besøk


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
        badValue = getValues(bad) #henter nye verder fra CoT en gang i minuttet
        kjokkenValue = getValues(kjokken)
        stueValue = getValues(stue)

        badTidValue = getValues(badTid)
        kjokkenTidValue = getValues(kjokkenTid)
        stueTidValue = getValues(stueTid)

        badDusjValue = getValues(badDusj)
        kjokkenOvnValue = getValues(kjokkenOvn)
        gjesteValue = getValues(gjester)

        guestList = COT.to_list(1,gjesteValue,[1,1,1,1,1,1,1])
        del guestList[0]
        print(guestList)
        
        i = 0

    print(stueCountDownList)
    badCountDown, kjokkenCountDown, stueCountDownList = checkQueues(badValue,kjokkenValue,stueValue)
    timeNow = time.time()

    if badCountDown: #Sjekker om noen står i kø
        if (timerBad + int(str(badTidValue)[1])*5*60) < timeNow: #sjekker om tiden har gått ut
            badValue, badTidValue = popReturnList(badValue,badTidValue,bad,badTid,1) #sletter riktig verdi og returner verdi
            badDusjValue = extraChoicePop(badDusjValue,badDusj) #gjør det samme for liste for ekstra valg
            timerBad = timeNow #resetter timer
    else:
        timerBad = timeNow
    if kjokkenCountDown:
        if (timerKjokken + int(str(kjokkenTidValue)[1])*5*60) < timeNow: # sjekker om tiden har gått ut
            kjokkenValue, kjokkenTidValue = popReturnList(kjokkenValue,kjokkenTidValue,kjokken,kjokkenTid,1)
            kjokkenOvnValue = extraChoicePop(kjokkenOvnValue,kjokkenOvn)
            oppvaskKapasitet += 10 #legger til 10% av oppvaskkapasitet
            if oppvaskKapasitet == 100:
                oppvask["Value"] = 1 #setter oppvask signal til 1
                requests.put('https://circusofthings.com/WriteValue',
				    data=json.dumps(oppvask),headers={'Content-Type':'application/json'}) 
                oppvaskKapasitet = 0


            timerKjokken = timeNow #ny tid på timer
    else:
        timerKjokken = timeNow
    if stueCountDownList[0] == True: #sjekker om man skal telle ned i første posisjon i stuelista
        if (timerStue1 + int(str(stueTidValue)[1])*15*60) < timeNow:
            stueValue, stueTidValue = popReturnList(stueValue,stueTidValue,stue,stueTid,1)
            timerStue2 = timerStue1 + int(str(stueTidValue)[1])*15*60 #flytter timerene en posisjon ned
            timerStue3 = timerStue2 + int(str(stueTidValue)[2])*15*60
            timerStue1 = timeNow
    else:
        timerStue1 = timeNow

    if stueCountDownList[1] == True:
        if (timerStue2 + int(str(stueTidValue)[2])*15*60) < timeNow:
            stueValue, stueTidValue = popReturnList(stueValue,stueTidValue,stue,stueTid,2)
            timerStue3 = timerStue2 + int(str(stueTidValue)[3])*15*60 #flytter timer 3 riktig posisjon
            timerStue2 = timeNow

            print(stueTidValue)
            print(guestList)
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
