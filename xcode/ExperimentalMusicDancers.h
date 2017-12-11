//
//  ExperimentalMusicDancers.h
//  MagneticGardel
//
//  Created by courtney on 11/17/17.
//
//

#ifndef ExperimentalMusicDancers_h
#define ExperimentalMusicDancers_h

namespace InteractiveTango
{
    class ExperimentalDanceFloor : public DanceFloor
    {
    protected:
        std::vector<MelodyGenerator> generators;
        MelodyGenerator follower_gen, leader_gen;
        FactorOracle leaderfo, followerfo;
        MelodySection *leaderMelody;
    public:
        ExperimentalDanceFloor(BeatTiming *timer) : DanceFloor(timer)
        {
            if(player != NULL) delete player;
            player = new ExperimentalMusicPlayer();
        };
        
        virtual void loadCourtneyTangoSongNumberOneOrnaments()
        {
            //just in case -- TODO: refactor so that this is not necessary
            std::cout << "Cannot load Fragments ornaments -- Not relevant for this dance floor!\n" ;
        };
        
        void leaderFakeStep()
        {
            couples[0]->getFollower()->getOnsets()->createFakeStep();
        }
        
        void followerFakeStep()
        {
            couples[0]->getLeader()->getOnsets()->createFakeStep();
        }
        
        void loadSong(BeatTiming *timer)
        {
            melody->addSchema( mMappingSchemas[MELODY_BS] );  //not using to choose melodies at the MOMENT //choosing again 8/2014
            mMappingSchemas[MELODY_BS]->setName("Follower Busy Sparse");
            melody->addSchema( mMappingSchemas[MELODY_INSTRUMENT_PR] );
            
            //leader once controlled only the harmonyr
            //                accompaniment->addSchema( mMappingSchemas[THICK_THIN] ); //7-29 deleting for now... busy/spare more accurate... I think... look at later...
            leaderMelody->addSchema( mMappingSchemas[ACCOMPANIMENT_BS] );
            mMappingSchemas[ACCOMPANIMENT_BS]->setName("Leader Busy Sparse");
            leaderMelody->addSchema( mMappingSchemas[ACCOMPANIMENT_INSTRUMENT_PR] );
            
            ((ExperimentalMusicPlayer *) player)->addGeneratedMelodySection( (GeneratedMelodySection *) melody );
            ((ExperimentalMusicPlayer *) player)->addGeneratedMelodySection( (GeneratedMelodySection *) leaderMelody );
            
            follower_gen.setMelodySection(melody);
            leader_gen.setMelodySection(leaderMelody);
//            
//            follower_gen.turnOn1to1();
//            leader_gen.turnOn1to1();

            //player->addAccompaniment( accompaniment ); // not yet implemented
        };
        
        virtual void loadCourtneyTangoSongNumberOneIntoPlayer()
        {
            //just in case -- TODO: refactor so that this is not necessary
            std::cout << "Cannot load Fragments song -- Not relevant for this dance floor!\n" ;
        };
        
        void loadGeneratedSong()
        {
            destroyCurSong();
            if( couples.size() <= 0 )
            {
                std::cout << "Warning! No couples on dance floor. Cannot load song! ";
            }
            
            //create new melody generator section
            
            leaderfo.train("/Users/courtney/Documents/Interactive Tango Milonga/EMTango_Melody/emtango.v3.mid", 2);
            leader_gen.addGeneratorAlgorithm(&leaderfo);
            generators.push_back(leader_gen);
            
            followerfo.train("/Users/courtney/Documents/Interactive Tango Milonga/EMTango_Melody/emtango.v3.mid", 1);
            follower_gen.addGeneratorAlgorithm(&followerfo);
            generators.push_back(follower_gen);
            
            
            melody = new GeneratedMelodySection( mTimer, melodyOnsetDancer->getOnsets(), &follower_gen, &instruments );
            leaderMelody = new GeneratedMelodySection( mTimer, couples[0]->getLeader()->getOnsets(), &leader_gen, &instruments ); //added leader...
            
            
            accompaniment = NULL; //not implemented yet -- new AccompanimentPorUnaCabeza( mTimer, &instruments );
            
            for( int i=0; i<couples.size(); i++ )
                couples[i]->setMotionAnalysisParamsForSong(TangoEntity::SongIDs::FRAGMENTS); //just use settings for fragments for now
            
            loadSong(mTimer);
            selectedSong = TangoEntity::SongIDs::GENERATED_PIAZZOLLA_TEST;
            std::cout << "Loaded Generated Melody song\n";
        };
        
        void loadPorUnaCabezaIntoPlayer()
        {
            //just in case -- TODO: refactor so that this is not necessary
            std::cout << "Cannot load Por Una Cabeza song -- Not relevant for this dance floor!\n" ;
        }
        
        
         
    };

        
};





#endif /* ExperimentalMusicDancers_h */
