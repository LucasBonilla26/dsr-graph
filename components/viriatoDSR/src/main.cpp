/*
 *    Copyright (C) 2021 by YOUR NAME HERE
 *
 *    This file is part of RoboComp
 *
 *    RoboComp is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    RoboComp is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RoboComp.  If not, see <http://www.gnu.org/licenses/>.
 */


/** \mainpage RoboComp::viriatoDSR
 *
 * \section intro_sec Introduction
 *
 * The viriatoDSR component...
 *
 * \section interface_sec Interface
 *
 * interface...
 *
 * \section install_sec Installation
 *
 * \subsection install1_ssec Software depencences
 * ...
 *
 * \subsection install2_ssec Compile and install
 * cd viriatoDSR
 * <br>
 * cmake . && make
 * <br>
 * To install:
 * <br>
 * sudo make install
 *
 * \section guide_sec User guide
 *
 * \subsection config_ssec Configuration file
 *
 * <p>
 * The configuration file etc/config...
 * </p>
 *
 * \subsection execution_ssec Execution
 *
 * Just: "${PATH_TO_BINARY}/viriatoDSR --Ice.Config=${PATH_TO_CONFIG_FILE}"
 *
 * \subsection running_ssec Once running
 *
 * ...
 *
 */
#include <signal.h>

// QT includes
#include <QtCore>
#include <QtWidgets>

// ICE includes
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Ice/Application.h>

#include <rapplication/rapplication.h>
#include <sigwatch/sigwatch.h>
#include <qlog/qlog.h>

#include "config.h"
#include "genericmonitor.h"
#include "genericworker.h"
#include "specificworker.h"
#include "specificmonitor.h"
#include "commonbehaviorI.h"

#include <camerargbdsimplepubI.h>
#include <jointmotorpubI.h>
#include <kinovaarmpubI.h>
#include <laserpubI.h>
#include <omnirobotpubI.h>

#include <CameraRGBDSimple.h>
#include <GenericBase.h>
#include <JointMotor.h>



class viriatoDSR : public RoboComp::Application
{
public:
	viriatoDSR (QString prfx, bool startup_check) { prefix = prfx.toStdString(); this->startup_check_flag=startup_check; }
private:
	void initialize();
	std::string prefix;
	TuplePrx tprx;
	bool startup_check_flag = false;

public:
	virtual int run(int, char*[]);
};

void ::viriatoDSR::initialize()
{
	// Config file properties read example
	// configGetString( PROPERTY_NAME_1, property1_holder, PROPERTY_1_DEFAULT_VALUE );
	// configGetInt( PROPERTY_NAME_2, property1_holder, PROPERTY_2_DEFAULT_VALUE );
}

int ::viriatoDSR::run(int argc, char* argv[])
{
#ifdef USE_QTGUI
	QApplication a(argc, argv);  // GUI application
#else
	QCoreApplication a(argc, argv);  // NON-GUI application
#endif


	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGHUP);
	sigaddset(&sigs, SIGINT);
	sigaddset(&sigs, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sigs, 0);

	UnixSignalWatcher sigwatch;
	sigwatch.watchForSignal(SIGINT);
	sigwatch.watchForSignal(SIGTERM);
	QObject::connect(&sigwatch, SIGNAL(unixSignal(int)), &a, SLOT(quit()));

	int status=EXIT_SUCCESS;

	RoboCompCoppeliaUtils::CoppeliaUtilsPrxPtr coppeliautils_proxy;
	RoboCompFullPoseEstimation::FullPoseEstimationPrxPtr fullposeestimation_proxy;
	RoboCompLaser::LaserPrxPtr laser_proxy;
	RoboCompOmniRobot::OmniRobotPrxPtr omnirobot_proxy;

	string proxy, tmp;
	initialize();

	try
	{
		if (not GenericMonitor::configGetString(communicator(), prefix, "CoppeliaUtilsProxy", proxy, ""))
		{
			cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy CoppeliaUtilsProxy\n";
		}
		coppeliautils_proxy = Ice::uncheckedCast<RoboCompCoppeliaUtils::CoppeliaUtilsPrx>( communicator()->stringToProxy( proxy ) );
	}
	catch(const Ice::Exception& ex)
	{
		cout << "[" << PROGRAM_NAME << "]: Exception creating proxy CoppeliaUtils: " << ex;
		return EXIT_FAILURE;
	}
	rInfo("CoppeliaUtilsProxy initialized Ok!");


	try
	{
		if (not GenericMonitor::configGetString(communicator(), prefix, "FullPoseEstimationProxy", proxy, ""))
		{
			cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy FullPoseEstimationProxy\n";
		}
		fullposeestimation_proxy = Ice::uncheckedCast<RoboCompFullPoseEstimation::FullPoseEstimationPrx>( communicator()->stringToProxy( proxy ) );
	}
	catch(const Ice::Exception& ex)
	{
		cout << "[" << PROGRAM_NAME << "]: Exception creating proxy FullPoseEstimation: " << ex;
		return EXIT_FAILURE;
	}
	rInfo("FullPoseEstimationProxy initialized Ok!");


	try
	{
		if (not GenericMonitor::configGetString(communicator(), prefix, "LaserProxy", proxy, ""))
		{
			cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy LaserProxy\n";
		}
		laser_proxy = Ice::uncheckedCast<RoboCompLaser::LaserPrx>( communicator()->stringToProxy( proxy ) );
	}
	catch(const Ice::Exception& ex)
	{
		cout << "[" << PROGRAM_NAME << "]: Exception creating proxy Laser: " << ex;
		return EXIT_FAILURE;
	}
	rInfo("LaserProxy initialized Ok!");


	try
	{
		if (not GenericMonitor::configGetString(communicator(), prefix, "OmniRobotProxy", proxy, ""))
		{
			cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy OmniRobotProxy\n";
		}
		omnirobot_proxy = Ice::uncheckedCast<RoboCompOmniRobot::OmniRobotPrx>( communicator()->stringToProxy( proxy ) );
	}
	catch(const Ice::Exception& ex)
	{
		cout << "[" << PROGRAM_NAME << "]: Exception creating proxy OmniRobot: " << ex;
		return EXIT_FAILURE;
	}
	rInfo("OmniRobotProxy initialized Ok!");


	IceStorm::TopicManagerPrxPtr topicManager;
	try
	{
		topicManager = topicManager = Ice::checkedCast<IceStorm::TopicManagerPrx>(communicator()->propertyToProxy("TopicManager.Proxy"));
		if (!topicManager)
		{
		    cout << "[" << PROGRAM_NAME << "]: TopicManager.Proxy not defined in config file."<<endl;
		    cout << "	 Config line example: TopicManager.Proxy=IceStorm/TopicManager:default -p 9999"<<endl;
	        return EXIT_FAILURE;
		}
	}
	catch (const Ice::Exception &ex)
	{
		cout << "[" << PROGRAM_NAME << "]: Exception: 'rcnode' not running: " << ex << endl;
		return EXIT_FAILURE;
	}

	tprx = std::make_tuple(coppeliautils_proxy,fullposeestimation_proxy,laser_proxy,omnirobot_proxy);
	SpecificWorker *worker = new SpecificWorker(tprx, startup_check_flag);
	//Monitor thread
	SpecificMonitor *monitor = new SpecificMonitor(worker,communicator());
	QObject::connect(monitor, SIGNAL(kill()), &a, SLOT(quit()));
	QObject::connect(worker, SIGNAL(kill()), &a, SLOT(quit()));
	monitor->start();

	if ( !monitor->isRunning() )
		return status;

	while (!monitor->ready)
	{
		usleep(10000);
	}

	try
	{
		try {
			// Server adapter creation and publication
			if (not GenericMonitor::configGetString(communicator(), prefix, "CommonBehavior.Endpoints", tmp, "")) {
				cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy CommonBehavior\n";
			}
			Ice::ObjectAdapterPtr adapterCommonBehavior = communicator()->createObjectAdapterWithEndpoints("commonbehavior", tmp);
			auto commonbehaviorI = std::make_shared<CommonBehaviorI>(monitor);
			adapterCommonBehavior->add(commonbehaviorI, Ice::stringToIdentity("commonbehavior"));
			adapterCommonBehavior->activate();
		}
		catch(const Ice::Exception& ex)
		{
			status = EXIT_FAILURE;

			cout << "[" << PROGRAM_NAME << "]: Exception raised while creating CommonBehavior adapter: " << endl;
			cout << ex;

		}



		// Server adapter creation and publication
		std::shared_ptr<IceStorm::TopicPrx> camerargbdsimplepub_topic;
		Ice::ObjectPrxPtr camerargbdsimplepub;
		try
		{
			if (not GenericMonitor::configGetString(communicator(), prefix, "CameraRGBDSimplePubTopic.Endpoints", tmp, ""))
			{
				cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy CameraRGBDSimplePubProxy";
			}
			Ice::ObjectAdapterPtr CameraRGBDSimplePub_adapter = communicator()->createObjectAdapterWithEndpoints("camerargbdsimplepub", tmp);
			RoboCompCameraRGBDSimplePub::CameraRGBDSimplePubPtr camerargbdsimplepubI_ =  std::make_shared <CameraRGBDSimplePubI>(worker);
			auto camerargbdsimplepub = CameraRGBDSimplePub_adapter->addWithUUID(camerargbdsimplepubI_)->ice_oneway();
			if(!camerargbdsimplepub_topic)
			{
				try {
					camerargbdsimplepub_topic = topicManager->create("CameraRGBDSimplePub");
				}
				catch (const IceStorm::TopicExists&) {
					//Another client created the topic
					try{
						cout << "[" << PROGRAM_NAME << "]: Probably other client already opened the topic. Trying to connect.\n";
						camerargbdsimplepub_topic = topicManager->retrieve("CameraRGBDSimplePub");
					}
					catch(const IceStorm::NoSuchTopic&)
					{
						cout << "[" << PROGRAM_NAME << "]: Topic doesn't exists and couldn't be created.\n";
						//Error. Topic does not exist
					}
				}
				catch(const IceUtil::NullHandleException&)
				{
					cout << "[" << PROGRAM_NAME << "]: ERROR TopicManager is Null. Check that your configuration file contains an entry like:\n"<<
					"\t\tTopicManager.Proxy=IceStorm/TopicManager:default -p <port>\n";
					return EXIT_FAILURE;
				}
				IceStorm::QoS qos;
				camerargbdsimplepub_topic->subscribeAndGetPublisher(qos, camerargbdsimplepub);
			}
			CameraRGBDSimplePub_adapter->activate();
		}
		catch(const IceStorm::NoSuchTopic&)
		{
			cout << "[" << PROGRAM_NAME << "]: Error creating CameraRGBDSimplePub topic.\n";
			//Error. Topic does not exist
		}


		// Server adapter creation and publication
		std::shared_ptr<IceStorm::TopicPrx> jointmotorpub_topic;
		Ice::ObjectPrxPtr jointmotorpub;
		try
		{
			if (not GenericMonitor::configGetString(communicator(), prefix, "JointMotorPubTopic.Endpoints", tmp, ""))
			{
				cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy JointMotorPubProxy";
			}
			Ice::ObjectAdapterPtr JointMotorPub_adapter = communicator()->createObjectAdapterWithEndpoints("jointmotorpub", tmp);
			RoboCompJointMotorPub::JointMotorPubPtr jointmotorpubI_ =  std::make_shared <JointMotorPubI>(worker);
			auto jointmotorpub = JointMotorPub_adapter->addWithUUID(jointmotorpubI_)->ice_oneway();
			if(!jointmotorpub_topic)
			{
				try {
					jointmotorpub_topic = topicManager->create("JointMotorPub");
				}
				catch (const IceStorm::TopicExists&) {
					//Another client created the topic
					try{
						cout << "[" << PROGRAM_NAME << "]: Probably other client already opened the topic. Trying to connect.\n";
						jointmotorpub_topic = topicManager->retrieve("JointMotorPub");
					}
					catch(const IceStorm::NoSuchTopic&)
					{
						cout << "[" << PROGRAM_NAME << "]: Topic doesn't exists and couldn't be created.\n";
						//Error. Topic does not exist
					}
				}
				catch(const IceUtil::NullHandleException&)
				{
					cout << "[" << PROGRAM_NAME << "]: ERROR TopicManager is Null. Check that your configuration file contains an entry like:\n"<<
					"\t\tTopicManager.Proxy=IceStorm/TopicManager:default -p <port>\n";
					return EXIT_FAILURE;
				}
				IceStorm::QoS qos;
				jointmotorpub_topic->subscribeAndGetPublisher(qos, jointmotorpub);
			}
			JointMotorPub_adapter->activate();
		}
		catch(const IceStorm::NoSuchTopic&)
		{
			cout << "[" << PROGRAM_NAME << "]: Error creating JointMotorPub topic.\n";
			//Error. Topic does not exist
		}


		// Server adapter creation and publication
		std::shared_ptr<IceStorm::TopicPrx> kinovaarmpub_topic;
		Ice::ObjectPrxPtr kinovaarmpub;
		try
		{
			if (not GenericMonitor::configGetString(communicator(), prefix, "KinovaArmPubTopic.Endpoints", tmp, ""))
			{
				cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy KinovaArmPubProxy";
			}
			Ice::ObjectAdapterPtr KinovaArmPub_adapter = communicator()->createObjectAdapterWithEndpoints("kinovaarmpub", tmp);
			RoboCompKinovaArmPub::KinovaArmPubPtr kinovaarmpubI_ =  std::make_shared <KinovaArmPubI>(worker);
			auto kinovaarmpub = KinovaArmPub_adapter->addWithUUID(kinovaarmpubI_)->ice_oneway();
			if(!kinovaarmpub_topic)
			{
				try {
					kinovaarmpub_topic = topicManager->create("KinovaArmPub");
				}
				catch (const IceStorm::TopicExists&) {
					//Another client created the topic
					try{
						cout << "[" << PROGRAM_NAME << "]: Probably other client already opened the topic. Trying to connect.\n";
						kinovaarmpub_topic = topicManager->retrieve("KinovaArmPub");
					}
					catch(const IceStorm::NoSuchTopic&)
					{
						cout << "[" << PROGRAM_NAME << "]: Topic doesn't exists and couldn't be created.\n";
						//Error. Topic does not exist
					}
				}
				catch(const IceUtil::NullHandleException&)
				{
					cout << "[" << PROGRAM_NAME << "]: ERROR TopicManager is Null. Check that your configuration file contains an entry like:\n"<<
					"\t\tTopicManager.Proxy=IceStorm/TopicManager:default -p <port>\n";
					return EXIT_FAILURE;
				}
				IceStorm::QoS qos;
				kinovaarmpub_topic->subscribeAndGetPublisher(qos, kinovaarmpub);
			}
			KinovaArmPub_adapter->activate();
		}
		catch(const IceStorm::NoSuchTopic&)
		{
			cout << "[" << PROGRAM_NAME << "]: Error creating KinovaArmPub topic.\n";
			//Error. Topic does not exist
		}


		// Server adapter creation and publication
		std::shared_ptr<IceStorm::TopicPrx> laserpub_topic;
		Ice::ObjectPrxPtr laserpub;
		try
		{
			if (not GenericMonitor::configGetString(communicator(), prefix, "LaserPubTopic.Endpoints", tmp, ""))
			{
				cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy LaserPubProxy";
			}
			Ice::ObjectAdapterPtr LaserPub_adapter = communicator()->createObjectAdapterWithEndpoints("laserpub", tmp);
			RoboCompLaserPub::LaserPubPtr laserpubI_ =  std::make_shared <LaserPubI>(worker);
			auto laserpub = LaserPub_adapter->addWithUUID(laserpubI_)->ice_oneway();
			if(!laserpub_topic)
			{
				try {
					laserpub_topic = topicManager->create("LaserPub");
				}
				catch (const IceStorm::TopicExists&) {
					//Another client created the topic
					try{
						cout << "[" << PROGRAM_NAME << "]: Probably other client already opened the topic. Trying to connect.\n";
						laserpub_topic = topicManager->retrieve("LaserPub");
					}
					catch(const IceStorm::NoSuchTopic&)
					{
						cout << "[" << PROGRAM_NAME << "]: Topic doesn't exists and couldn't be created.\n";
						//Error. Topic does not exist
					}
				}
				catch(const IceUtil::NullHandleException&)
				{
					cout << "[" << PROGRAM_NAME << "]: ERROR TopicManager is Null. Check that your configuration file contains an entry like:\n"<<
					"\t\tTopicManager.Proxy=IceStorm/TopicManager:default -p <port>\n";
					return EXIT_FAILURE;
				}
				IceStorm::QoS qos;
				laserpub_topic->subscribeAndGetPublisher(qos, laserpub);
			}
			LaserPub_adapter->activate();
		}
		catch(const IceStorm::NoSuchTopic&)
		{
			cout << "[" << PROGRAM_NAME << "]: Error creating LaserPub topic.\n";
			//Error. Topic does not exist
		}


		// Server adapter creation and publication
		std::shared_ptr<IceStorm::TopicPrx> omnirobotpub_topic;
		Ice::ObjectPrxPtr omnirobotpub;
		try
		{
			if (not GenericMonitor::configGetString(communicator(), prefix, "OmniRobotPubTopic.Endpoints", tmp, ""))
			{
				cout << "[" << PROGRAM_NAME << "]: Can't read configuration for proxy OmniRobotPubProxy";
			}
			Ice::ObjectAdapterPtr OmniRobotPub_adapter = communicator()->createObjectAdapterWithEndpoints("omnirobotpub", tmp);
			RoboCompOmniRobotPub::OmniRobotPubPtr omnirobotpubI_ =  std::make_shared <OmniRobotPubI>(worker);
			auto omnirobotpub = OmniRobotPub_adapter->addWithUUID(omnirobotpubI_)->ice_oneway();
			if(!omnirobotpub_topic)
			{
				try {
					omnirobotpub_topic = topicManager->create("OmniRobotPub");
				}
				catch (const IceStorm::TopicExists&) {
					//Another client created the topic
					try{
						cout << "[" << PROGRAM_NAME << "]: Probably other client already opened the topic. Trying to connect.\n";
						omnirobotpub_topic = topicManager->retrieve("OmniRobotPub");
					}
					catch(const IceStorm::NoSuchTopic&)
					{
						cout << "[" << PROGRAM_NAME << "]: Topic doesn't exists and couldn't be created.\n";
						//Error. Topic does not exist
					}
				}
				catch(const IceUtil::NullHandleException&)
				{
					cout << "[" << PROGRAM_NAME << "]: ERROR TopicManager is Null. Check that your configuration file contains an entry like:\n"<<
					"\t\tTopicManager.Proxy=IceStorm/TopicManager:default -p <port>\n";
					return EXIT_FAILURE;
				}
				IceStorm::QoS qos;
				omnirobotpub_topic->subscribeAndGetPublisher(qos, omnirobotpub);
			}
			OmniRobotPub_adapter->activate();
		}
		catch(const IceStorm::NoSuchTopic&)
		{
			cout << "[" << PROGRAM_NAME << "]: Error creating OmniRobotPub topic.\n";
			//Error. Topic does not exist
		}


		// Server adapter creation and publication
		cout << SERVER_FULL_NAME " started" << endl;

		// User defined QtGui elements ( main window, dialogs, etc )

		#ifdef USE_QTGUI
			//ignoreInterrupt(); // Uncomment if you want the component to ignore console SIGINT signal (ctrl+c).
			a.setQuitOnLastWindowClosed( true );
		#endif
		// Run QT Application Event Loop
		a.exec();

		try
		{
			std::cout << "Unsubscribing topic: omnirobotpub " <<std::endl;
			omnirobotpub_topic->unsubscribe( omnirobotpub );
		}
		catch(const Ice::Exception& ex)
		{
			std::cout << "ERROR Unsubscribing topic: omnirobotpub " << ex.what()<<std::endl;
		}


		status = EXIT_SUCCESS;
	}
	catch(const Ice::Exception& ex)
	{
		status = EXIT_FAILURE;

		cout << "[" << PROGRAM_NAME << "]: Exception raised on main thread: " << endl;
		cout << ex;

	}
	#ifdef USE_QTGUI
		a.quit();
	#endif

	status = EXIT_SUCCESS;
	monitor->terminate();
	monitor->wait();
	delete worker;
	delete monitor;
	return status;
}

int main(int argc, char* argv[])
{
	string arg;

	// Set config file
	QString configFile("etc/config");
	bool startup_check_flag = false;
	QString prefix("");
	if (argc > 1)
	{

		// Search in argument list for arguments
		QString startup = QString("--startup-check");
		QString initIC = QString("--Ice.Config=");
		QString prfx = QString("--prefix=");
		for (int i = 0; i < argc; ++i)
		{
			arg = argv[i];
			if (arg.find(startup.toStdString(), 0) != std::string::npos)
			{
				startup_check_flag = true;
				cout << "Startup check = True"<< endl;
			}
			else if (arg.find(prfx.toStdString(), 0) != std::string::npos)
			{
				prefix = QString::fromStdString(arg).remove(0, prfx.size());
				if (prefix.size()>0)
					prefix += QString(".");
				printf("Configuration prefix: <%s>\n", prefix.toStdString().c_str());
			}
			else if (arg.find(initIC.toStdString(), 0) != std::string::npos)
			{
				configFile = QString::fromStdString(arg).remove(0, initIC.size());
				qDebug()<<__LINE__<<"Starting with config file:"<<configFile;
			}
			else if (i==1 and argc==2 and arg.find("--", 0) == std::string::npos)
			{
				configFile = QString::fromStdString(arg);
				qDebug()<<__LINE__<<QString::fromStdString(arg)<<argc<<arg.find("--", 0)<<"Starting with config file:"<<configFile;
			}
		}

	}
	::viriatoDSR app(prefix, startup_check_flag);

	return app.main(argc, argv, configFile.toLocal8Bit().data());
}
