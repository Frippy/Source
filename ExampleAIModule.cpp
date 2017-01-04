#include "ExampleAIModule.h" 
using namespace BWAPI;
bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;
std::vector<Unit*> mySCV; 
std::vector<Unit*> myCC;
int machineShopTilePosX, machineShopTilePosY;
int reservedMinerals;
int reservedGas;
std::vector<Unit*> supplyList;
Unit* closestMineral=NULL;
bool geyser = false;
bool start = false;
bool enrouteSuppDep = false;
bool enrouteBarracks = false;
bool enrouteVespine = false;
bool enrouteAcademy = false;
bool enrouteFactory = false;
bool jumping = false;
std::vector<int> tempSCV;
int numbr = 0;
int barrWait = 0;
int factWait = 0;
int geyserTimer = 0;
TilePosition posSuppDep;
TilePosition posBarracks;
TilePosition posVesp;
TilePosition posAcademy;
TilePosition posFactory;
std::vector<Unit*> barackObama;
std::vector<Unit*> marinus;
std::vector<Unit*> medicus;
std::vector<Unit*> factories;
std::vector<Unit*> tankus;
Unit* myGeyser = NULL;
int gasCV = 0;
int mineralLine = 0;
std::vector<Unit*> gasVec;
int buildingQueue;
bool startAcademy = false;
bool startFactory = false;
bool startMachineShop = false;
Unit* myAcademy = NULL;
Unit* myFactory = NULL;
bool constructingSuppDep = false;
bool constructingBarracks = false;
bool constructingAcademy = false;
bool constructingFactory = false;
bool hasBarracks = false;
bool hasAcademy = false;
bool hasFactory = false;
bool hasMachineShop = false;

TilePosition getBuildTile(Unit* builder, UnitType buildingType, TilePosition aroundTile){
    TilePosition ret = TilePositions::Invalid;
    int maxDist = 3;
    int stopDist = 40;
    if(buildingType.isRefinery()){
        for(std::set<Unit*>::const_iterator i = Broodwar->getNeutralUnits().begin(); i != Broodwar->getNeutralUnits().end(); i++){    // Checks if the type to build is a refinery,
            if((*i)->getType() == UnitTypes::Resource_Vespene_Geyser                                                                // searches for a vespene geyser and then returns that position.
                && (abs((*i)->getTilePosition().x() - aroundTile.x()) < stopDist)
                && (abs((*i)->getTilePosition().y() - aroundTile.y()) < stopDist)){
				if(!(*i)->isVisible())
					continue;
                return (*i)->getTilePosition();
            }
        }
    }
    while(maxDist < stopDist){
        for(int i = aroundTile.x() - maxDist; i <= aroundTile.x() + maxDist; i++){
            for(int j = aroundTile.y() - maxDist; j <= aroundTile.y() + maxDist; j++){
                if(Broodwar->canBuildHere(builder, TilePosition(i,j), buildingType, false)){
					bool unitsInWay = false;
					for(std::set<Unit*>::const_iterator u = Broodwar->self()->getUnits().begin(); u != Broodwar->self()->getUnits().end(); u++){
						if((*u)->getID() == (builder)->getID()) 
						continue;
						if((abs((*u)->getTilePosition().x() - i) < 4)
							&& (abs((*u)->getTilePosition().y() - j) < 4)) 
						unitsInWay = true;
					}
					if(!unitsInWay)
					{
						if(buildingType == UnitTypes::Terran_Factory){
							if(Broodwar->canBuildHere(builder, TilePosition(i + buildingType.tileWidth(),j + buildingType.tileHeight()-UnitTypes::Terran_Machine_Shop.tileHeight()),UnitTypes::Terran_Machine_Shop,false))
							{
								machineShopTilePosX = i + buildingType.tileWidth(); 
								machineShopTilePosY = j + buildingType.tileHeight() - UnitTypes::Terran_Machine_Shop.tileHeight();
								return TilePosition(i,j);
							}
						}
						else
						{
							if(hasFactory && !hasMachineShop && i >= machineShopTilePosX && i < machineShopTilePosX + UnitTypes::Terran_Machine_Shop.tileWidth() && machineShopTilePosY + buildingType.tileHeight() > machineShopTilePosY && j < machineShopTilePosY + UnitTypes::Terran_Machine_Shop.tileHeight())
							{
								continue;
							}
							Broodwar->sendText("i %d j %d",i,j);
							return TilePosition(i,j);
						}
					}
                }
            }
        }
		if(buildingType == UnitTypes::Terran_Factory)
			maxDist += 3;
		else
			maxDist += 2;
    }

    return ret;
}

void initializeSupplyList(Unit* t)
{
	bool found = false;
	for(int i = 0; i<8;i++)
	{
		found = false;
		Unit* closestMineral=NULL;
		for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
		{
			if (closestMineral==NULL || t->getDistance(*m)<t->getDistance(closestMineral))
			{	
				found = false;
				for(int j = 0;j < supplyList.size();j++)
				{
					if((*m)->getID() == supplyList.at(j)->getID())
					{
						found = true;
						break;
					}
				}
				if(!found)
				{
					closestMineral=*m;
				}
			}
		}
		if(closestMineral == NULL)
			Broodwar->sendText("Something went wrong");	
		supplyList.push_back(closestMineral);
	}
//	Broodwar->sendText("%d",supplyList.size());
}

void sendToMinerals(Unit* t)
{
	//Broodwar->printf("Send worker %d to mineral %d", supplyList.at(iterator)->getID());
	if(supplyList.size()==8)
	{
		t->rightClick(supplyList.at(mineralLine++));
		if(mineralLine == 8)
			mineralLine = 0;
	}
}

//This is the startup method. It is called once
//when a new game has been started with the bot.
void ExampleAIModule::onStart()
{
	Broodwar->setLocalSpeed(0);
	tempSCV.push_back(NULL);
	tempSCV.push_back(NULL);
	tempSCV.push_back(NULL);
	tempSCV.push_back(NULL);
	tempSCV.push_back(NULL);
	Broodwar->sendText("Hello world!");
	//Enable flags
	Broodwar->enableFlag(Flag::UserInput);
	//Uncomment to enable complete map information
	Broodwar->enableFlag(Flag::CompleteMapInformation);

	//Start analyzing map data
	BWTA::readMap();
	analyzed=false;
	analysis_just_finished=false;
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL); //Threaded version
	AnalyzeThread();
	bool found = false;
    //Send each worker to the mineral field that is closest to it
    for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end() && !found;i++)
    {
		if ((*i)->getType().isWorker())
		{
			if(supplyList.size()==0)
			{
				found = true;
				initializeSupplyList(*i);
			}
		}
	}
}

//Called when a game is ended.
//No need to change this.
void ExampleAIModule::onEnd(bool isWinner)
{
	if (isWinner)
	{
		Broodwar->sendText("I won!");
	}
}

//Finds a guard point around the home base.
//A guard point is the center of a chokepoint surrounding
//the region containing the home base.
Position ExampleAIModule::findGuardPoint()
{
	//Get the chokepoints linked to our home region
	std::set<BWTA::Chokepoint*> chokepoints = home->getChokepoints();
	double min_length = 10000;
	BWTA::Chokepoint* choke = NULL;

	//Iterate through all chokepoints and look for the one with the smallest gap (least width)
	for(std::set<BWTA::Chokepoint*>::iterator c = chokepoints.begin(); c != chokepoints.end(); c++)
	{
		double length = (*c)->getWidth();
		if (length < min_length || choke==NULL)
		{
			min_length = length;
			choke = *c;
		}
	}

	return choke->getCenter();
}

int findSCV(int num)
{
	if(num==0)
	{
		for(int i = 0; i<4;i++)
		{
			if(mySCV.at(i)->getHitPoints() > 0 && !mySCV.at(i)->isConstructing() && !mySCV.at(i)->isGatheringGas())
				return i;
		}
		if(true)
		{
			Broodwar->sendText("FATAL ERROR");
			return 0;
		}
	}
	else if(num==1)
	{
		for(int i = 5; i<7;i++)
		{
			if(mySCV.at(i)->getHitPoints() > 0 && !mySCV.at(i)->isConstructing() && !mySCV.at(i)->isGatheringGas())
				return i;
		}
		if(true)
		{
			Broodwar->sendText("FATAL ERROR");
			return 5;
		}
	}
	else if(num==2)
	{
		for(int i = 7; i<9;i++)
		{
			if(mySCV.at(i)->getHitPoints() > 0 && !mySCV.at(i)->isConstructing() && !mySCV.at(i)->isGatheringGas())
				return i;
		}
		if(true)
		{
			Broodwar->sendText("FATAL ERROR");
			return 9;
		}
	}
	else if(num==3)
	{
		for(int i = 9; i<11;i++)
		{
			if(mySCV.at(i)->getHitPoints() > 0 && !mySCV.at(i)->isConstructing() && !mySCV.at(i)->isGatheringGas())
				return i;
		}
		if(true)
		{
			Broodwar->sendText("FATAL ERROR");
			return 9;
		}
	}
	else if(num==4)
	{
		for(int i = 11; i<13;i++)
		{
			if(mySCV.at(i)->getHitPoints() > 0 && !mySCV.at(i)->isConstructing() && !mySCV.at(i)->isGatheringGas())
				return i;
		}
		if(true)
		{
			Broodwar->sendText("FATAL ERROR");
			return 9;
		}
	}
	/*for(int i = 0; i < mySCV.size();i++)
	{
		if
		if(!mySCV.at(i)->isGatheringGas() && !mySCV.at(i)->isConstructing() && mySCV.at(i)->getHitPoints() > 0 && !mySCV.at(i)->isCarryingMinerals())
		{
			found = false;
			for(int j=0;j<tempSCV.size();j++)
			{
				if(j!=num && tempSCV.at(j) != NULL && tempSCV.at(j) == i)
					found = true;
			}
			if(!found)
				return i;
		}
	}*/
}

void buildSCV()
{
	if(myCC.size()==0)
	{}
	else if(Broodwar->self()->minerals() - reservedMinerals >= 50 && myCC.at(0)->getTrainingQueue().size() == 0 && mySCV.size() <19)
	{
		myCC.at(0)->train(UnitTypes::Terran_SCV);
	}
}
void buildMarine()
{
	int iterator = 0;
	for(int i = 0; i < barackObama.size();i++)
	{
		if(Broodwar->self()->minerals() - reservedMinerals - iterator >= 50 && barackObama.at(i)->getTrainingQueue().size() == 0)
		{
			if(i % 2 == 0)
			{
				iterator+=50;
				barackObama.at(i)->train(UnitTypes::Terran_Marine);
			}
		}
	}
}

void buildMedic()
{
	int iteratorMinerals = 0;
	int iteratorGas = 0;
	for(int i = 0; i < barackObama.size();i++)
	{
		if(Broodwar->self()->minerals() - reservedMinerals - iteratorMinerals >= UnitTypes::Terran_Medic.mineralPrice() && Broodwar->self()->gas() - reservedGas - iteratorGas >= UnitTypes::Terran_Medic.gasPrice() && barackObama.at(i)->getTrainingQueue().size() == 0
			&& hasAcademy && !constructingAcademy)
		{
			if(i % 2 > 0)
			{
				iteratorMinerals += UnitTypes::Terran_Medic.mineralPrice();
				iteratorGas += UnitTypes::Terran_Medic.gasPrice();
				barackObama.at(i)->train(UnitTypes::Terran_Medic);
			}
		}
	}
}

void buildTank()
{
	int iteratorMinerals = 0;
	int iteratorGas = 0;
	for(int i = 0; i < factories.size();i++)
	{
		if(Broodwar->self()->minerals() - reservedMinerals - iteratorMinerals >= 150 && Broodwar->self()->gas() - reservedGas - iteratorGas >= 100 && factories.at(i)->getTrainingQueue().size() == 0)
		{
			iteratorMinerals += 150;
			iteratorGas += 100;
			factories.at(i)->train(UnitTypes::Terran_Siege_Tank_Tank_Mode);
		}
	}
}

void buildSupplyDepot()
{
	if(enrouteSuppDep)
	{
		if(mySCV.at(tempSCV.at(0))->getHitPoints() <= 0)
		{
			enrouteSuppDep = false;
		}
		mySCV.at(tempSCV.at(0))->build(posSuppDep, UnitTypes::Terran_Supply_Depot);
		

	}
	else if(Broodwar->self()->supplyUsed() + 4 + barackObama.size() >= Broodwar->self()->supplyTotal() && !constructingSuppDep)
	{
		Broodwar->sendText("Supply Depot");
		tempSCV[0] = findSCV(0);
		posSuppDep = getBuildTile(mySCV.at(tempSCV[0]),UnitTypes::Terran_Supply_Depot,Broodwar->self()->getStartLocation());
		reservedMinerals+= UnitTypes::Terran_Supply_Depot.mineralPrice();
		enrouteSuppDep = true;
	}
}

void buildBarracks()
{
	if(enrouteBarracks)
	{
		if(mySCV.at(tempSCV.at(1))->getHitPoints() <= 0)
		{
			enrouteBarracks = false;
		}
		mySCV[tempSCV[1]]->build(posBarracks, UnitTypes::Terran_Barracks);
		if(enrouteSuppDep)
		{
			sendToMinerals(mySCV[tempSCV[1]]);
			reservedMinerals -= UnitTypes::Terran_Barracks.mineralPrice();
			enrouteBarracks = false;
		}
	}
	else if(Broodwar->self()->supplyUsed() >= 22 + barrWait && reservedMinerals == 0 && barackObama.size()<3 && !enrouteSuppDep && !constructingBarracks)
	{

		Broodwar->sendText("Barracks");
		tempSCV[1] = findSCV(1);
		posBarracks = getBuildTile(mySCV.at(tempSCV[1]),UnitTypes::Terran_Barracks,Broodwar->self()->getStartLocation());
		reservedMinerals += UnitTypes::Terran_Barracks.mineralPrice();
		enrouteBarracks = true;
	}
}
void buildAcademy()
{
	if(enrouteAcademy)
	{
		if(mySCV.at(tempSCV.at(3))->getHitPoints() <= 0)
		{
			enrouteAcademy = false;
		}
		mySCV[tempSCV[3]]->build(posAcademy, UnitTypes::Terran_Academy);
		if(enrouteSuppDep)
		{
			sendToMinerals(mySCV[tempSCV[3]]);
			reservedMinerals -= UnitTypes::Terran_Academy.mineralPrice();
			enrouteAcademy = false;
		}
	}
	else if(reservedMinerals == 0 && myAcademy == NULL && startAcademy && !enrouteSuppDep && !hasAcademy && hasBarracks && hasFactory)
	{
		Broodwar->sendText("Academy");
		tempSCV[3] = findSCV(3);
		posAcademy = getBuildTile(mySCV[tempSCV[3]],UnitTypes::Terran_Academy,Broodwar->self()->getStartLocation());
		reservedMinerals += UnitTypes::Terran_Academy.mineralPrice();
		enrouteAcademy = true;
	}
}
void buildFactory()
{
	if(enrouteFactory)
	{
		if(mySCV[tempSCV.at(4)]->getHitPoints() <= 0)
		{
			enrouteFactory = false;
		}
		mySCV[tempSCV[4]]->build(posFactory, UnitTypes::Terran_Factory);
		if(enrouteSuppDep)
		{
			sendToMinerals(mySCV[tempSCV[4]]);
			reservedMinerals -= UnitTypes::Terran_Factory.mineralPrice();
			enrouteFactory = false;
		}
	}
	else if(reservedMinerals == 0 && myFactory == NULL && startFactory && !enrouteSuppDep && !hasFactory && hasBarracks)
	{
		Broodwar->sendText("Factory");
		tempSCV[4] = findSCV(4);
		posFactory = getBuildTile(mySCV[tempSCV[4]],UnitTypes::Terran_Factory,Broodwar->self()->getStartLocation());
		reservedMinerals += UnitTypes::Terran_Factory.mineralPrice();
		enrouteFactory = true;
	}
}
void buildMachineShop()
{
	if(hasFactory && !constructingFactory && !hasMachineShop)
	{
		myFactory->buildAddon(UnitTypes::Terran_Machine_Shop);
		Broodwar->sendText("MachineShop");
		reservedMinerals += UnitTypes::Terran_Machine_Shop.mineralPrice();
		reservedGas += UnitTypes::Terran_Machine_Shop.gasPrice();
	}
}
void buildRefinery()
{
	if(enrouteVespine)
	{
		if(mySCV.at(tempSCV.at(2))->getHitPoints() <= 0)
		{
			enrouteVespine = false;
		}

		mySCV[tempSCV[2]]->build(posVesp, UnitTypes::Terran_Refinery);

	}
	else if(Broodwar->self()->supplyUsed() == 24 && reservedMinerals == 0)
	{
		tempSCV[2] = findSCV(2);
		posVesp = getBuildTile(mySCV.at(tempSCV[2]),UnitTypes::Terran_Refinery,Broodwar->self()->getStartLocation());
		mySCV[tempSCV[2]]->build(posVesp,  UnitTypes::Terran_Refinery);
		reservedMinerals+= UnitTypes::Terran_Refinery.mineralPrice();
		enrouteVespine = true;
	}
}

void sendToGeyser()
{
	if(myGeyser != NULL && myGeyser->isCompleted() && gasCV<3)
	{
		bool found = false;
		for(int i = 0; i< mySCV.size();i++)
		{
			if(!mySCV.at(i)->isConstructing() && !mySCV.at(i)->isCarryingMinerals() && !mySCV.at(i)->isGatheringGas())
			{
				for(int j = 0;j<gasVec.size();j++)
				{
					if(mySCV.at(i)->getID() == gasVec.at(j)->getID())
						found = true;
				}
				if(!found)
				{
					gasVec.push_back(mySCV.at(i));
					mySCV.at(i)->gather(myGeyser);
					gasCV++;
					break;
				}
			}
		}
	}
}
void ExampleAIModule::onFrame()
{
	if(supplyList.size()==0)
		initializeSupplyList(mySCV.at(0));
	//Broodwar->sendText("%d",UnitTypes::Terran_Refinery.mineralPrice());
	if(Broodwar->getFrameCount() % 50 == 0)
		Broodwar->sendText("%d",reservedMinerals);
	buildSCV();
	buildSupplyDepot();
	buildMarine();
	buildTank();
	buildBarracks();
	buildRefinery();
	buildAcademy();
	buildFactory();
	buildMachineShop();
	buildMedic();
	sendToGeyser();
	if(analyzed)
		drawTerrainData();
}

//Is called when text is written in the console window.
//Can be used to toggle stuff on and off.
void ExampleAIModule::onSendText(std::string text)
{
	if (text=="/show players")
	{
		showPlayers();
	}
	else if (text=="/show forces")
	{
		showForces();
	}
	else
	{
		Broodwar->printf("You typed '%s'!",text.c_str());
		Broodwar->sendText("%s",text.c_str());
	}
}

//Called when the opponent sends text messages.
//No need to change this.
void ExampleAIModule::onReceiveText(BWAPI::Player* player, std::string text)
{
	Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

//Called when a player leaves the game.
//No need to change this.
void ExampleAIModule::onPlayerLeft(BWAPI::Player* player)
{
	Broodwar->sendText("%s left the game.",player->getName().c_str());
}

//Called when a nuclear launch is detected.
//No need to change this.
void ExampleAIModule::onNukeDetect(BWAPI::Position target)
{
	if (target!=Positions::Unknown)
	{
		Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
	}
	else
	{
		Broodwar->printf("Nuclear Launch Detected");
	}
}

//No need to change this.
void ExampleAIModule::onUnitDiscover(BWAPI::Unit* unit)
{
	//Broodwar->sendText("A %s [%x] has been discovered at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

//No need to change this.
void ExampleAIModule::onUnitEvade(BWAPI::Unit* unit)
{
	//Broodwar->sendText("A %s [%x] was last accessible at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

//No need to change this.
void ExampleAIModule::onUnitShow(BWAPI::Unit* unit)
{
	//Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

//No need to change this.
void ExampleAIModule::onUnitHide(BWAPI::Unit* unit)
{
	//Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

//Called when a new unit has been created.
//Note: The event is called when the new unit is built, not when it
//has been finished.
void ExampleAIModule::onUnitCreate(BWAPI::Unit* unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		if(unit->getType() == UnitTypes::Terran_Supply_Depot)
		{
			reservedMinerals -= UnitTypes::Terran_Supply_Depot.mineralPrice();;
			Broodwar->sendText("Depot minus!");
			constructingSuppDep = true;
			enrouteSuppDep = false;
		}
		else if(unit->getType() == UnitTypes::Terran_Barracks)
		{
			hasBarracks = true;
			reservedMinerals -= UnitTypes::Terran_Barracks.mineralPrice();
			Broodwar->sendText("Barracks minus!");
			constructingBarracks = true;
			enrouteBarracks = false;
		}
		else if(unit->getType() == UnitTypes::Terran_Factory)
		{
			hasFactory = true;
			reservedMinerals -= UnitTypes::Terran_Factory.mineralPrice();
			reservedGas -= UnitTypes::Terran_Factory.gasPrice();
			Broodwar->sendText("Factory minus!");
			constructingFactory= true;
			enrouteFactory = false;
		}
		else if(unit->getType() == UnitTypes::Terran_Machine_Shop)
		{
			hasMachineShop = true;
			reservedMinerals -= UnitTypes::Terran_Machine_Shop.mineralPrice();
			reservedGas -= UnitTypes::Terran_Machine_Shop.gasPrice();
			Broodwar->sendText("Machine Shop minus!");
		}
		else if(unit->getType() == UnitTypes::Terran_Academy)
		{
			hasAcademy = true;
			reservedMinerals -= UnitTypes::Terran_Academy.mineralPrice();
			Broodwar->sendText("Academy minus!");
			constructingAcademy = true;
			enrouteAcademy = false;
		}

		Broodwar->sendText("A %s [%x] has been created at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	}
}

//Called when a unit has been destroyed.
void ExampleAIModule::onUnitDestroy(BWAPI::Unit* unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		Broodwar->sendText("My unit %s [%x] has been destroyed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	}
	else
	{
		Broodwar->sendText("Enemy unit %s [%x] has been destroyed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	}
}

//Only needed for Zerg units.
//No need to change this.
void ExampleAIModule::onUnitMorph(BWAPI::Unit* unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		if(unit->getType() == UnitTypes::Terran_Refinery)
		{
			gasCV++;
			gasVec.push_back(mySCV.at(tempSCV.at(2)));
			tempSCV.at(2) = NULL;
			myGeyser = unit;
			reservedMinerals -= UnitTypes::Terran_Refinery.mineralPrice();
			Broodwar->sendText("Refinery minus!");
		}
	}

	//Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

//No need to change this.
void ExampleAIModule::onUnitRenegade(BWAPI::Unit* unit)
{
	
	//Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}

//No need to change this.
void ExampleAIModule::onSaveGame(std::string gameName)
{
	Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

//Analyzes the map.
//No need to change this.
DWORD WINAPI AnalyzeThread()
{
	BWTA::analyze();

	//Self start location only available if the map has base locations
	if (BWTA::getStartLocation(BWAPI::Broodwar->self())!=NULL)
	{
		home = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
	}
	//Enemy start location only available if Complete Map Information is enabled.
	if (BWTA::getStartLocation(BWAPI::Broodwar->enemy())!=NULL)
	{
		enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
	}
	analyzed = true;
	analysis_just_finished = true;
	return 0;
}

//Prints some stats about the units the player has.
//No need to change this.
void ExampleAIModule::drawStats()
{
	std::set<Unit*> myUnits = Broodwar->self()->getUnits();
	Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
	std::map<UnitType, int> unitTypeCounts;
	for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++)
	{
		if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end())
		{
			unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
		}
		unitTypeCounts.find((*i)->getType())->second++;
	}
	int line=1;
	for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++)
	{
		Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
		line++;
	}
}

//Draws terrain data aroung regions and chokepoints.
//No need to change this.
void ExampleAIModule::drawTerrainData()
{
	//Iterate through all the base locations, and draw their outlines.
	for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();i++)
	{
		TilePosition p=(*i)->getTilePosition();
		Position c=(*i)->getPosition();
		//Draw outline of center location
		Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);
		//Draw a circle at each mineral patch
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getStaticMinerals().begin();j!=(*i)->getStaticMinerals().end();j++)
		{
			Position q=(*j)->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
		}
		//Draw the outlines of vespene geysers
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();j++)
		{
			TilePosition q=(*j)->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
		}
		//If this is an island expansion, draw a yellow circle around the base location
		if ((*i)->isIsland())
		{
			Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
		}
	}
	//Iterate through all the regions and draw the polygon outline of it in green.
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
	{
		BWTA::Polygon p=(*r)->getPolygon();
		for(int j=0;j<(int)p.size();j++)
		{
			Position point1=p[j];
			Position point2=p[(j+1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
		}
	}
	//Visualize the chokepoints with red lines
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
	{
		for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();c++)
		{
			Position point1=(*c)->getSides().first;
			Position point2=(*c)->getSides().second;
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
		}
	}
}

//Show player information.
//No need to change this.
void ExampleAIModule::showPlayers()
{
	std::set<Player*> players=Broodwar->getPlayers();
	for(std::set<Player*>::iterator i=players.begin();i!=players.end();i++)
	{
		Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
	}
}

//Show forces information.
//No need to change this.
void ExampleAIModule::showForces()
{
	std::set<Force*> forces=Broodwar->getForces();
	for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();i++)
	{
		std::set<Player*> players=(*i)->getPlayers();
		Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
		for(std::set<Player*>::iterator j=players.begin();j!=players.end();j++)
		{
			Broodwar->printf("  - Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
		}
	}
}

//Called when a unit has been completed, i.e. finished built.
void ExampleAIModule::onUnitComplete(BWAPI::Unit *unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		if(unit->getType().isWorker())
		{
			mySCV.push_back(unit);
			Broodwar->sendText("new SCV %d with ID %d", mySCV.size()-1,unit->getID());
			if(supplyList.size()==0)
				initializeSupplyList(unit);
			sendToMinerals(unit);
		}
		else if(unit->getType() == UnitTypes::Terran_Command_Center)
		{
			myCC.push_back(unit);
		}
		else if(unit->getType() == UnitTypes::Terran_Supply_Depot)
		{
			sendToMinerals(mySCV.at(tempSCV.at(0)));
			tempSCV.at(0) = NULL;
			constructingSuppDep = false;
		}
		else if(unit->getType() == UnitTypes::Terran_Barracks)
		{
			barrWait = Broodwar->self()->supplyUsed() - 18;
			sendToMinerals(mySCV.at(tempSCV.at(1)));
			barackObama.push_back(unit);
			tempSCV.at(1) = NULL;
			startFactory = true;
			constructingBarracks = false;
		}
		else if(unit->getType() == UnitTypes::Terran_Factory)
		{
			//factWait = Broodwar->self()->supplyUsed() - 18;
			myFactory = unit;
			sendToMinerals(mySCV.at(tempSCV.at(4)));
			mySCV.at(4) = NULL;
			factories.push_back(unit);
			constructingFactory = false;
			startAcademy = true;
		}
		else if(unit->getType() == UnitTypes::Terran_Machine_Shop)
		{
			//factWait = Broodwar->self()->supplyUsed() - 18;
		}
		else if(unit->getType() == UnitTypes::Terran_Academy)
		{
			myAcademy = unit;
			sendToMinerals(mySCV.at(tempSCV.at(3)));
			mySCV.at(3) = NULL;
			constructingAcademy = false;
		}
		else if(unit->getType() == UnitTypes::Terran_Marine)
		{
			unit->attack(findGuardPoint());
			marinus.push_back(unit);
		}
		else if(unit->getType() == UnitTypes::Terran_Medic)
		{
			unit->attack(findGuardPoint());
			medicus.push_back(unit);
		}
		else if(unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode)
		{
			unit->attack(findGuardPoint());
			tankus.push_back(unit);
		}

	}

	//Broodwar->sendText("A %s [%x] has been completed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}




