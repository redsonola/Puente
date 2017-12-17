//
//  MelodyGenerator.h
//  MagneticGardel
//
//  Created by courtney on 11/14/17.
//
//

#ifndef MelodyGenerator_h
#define MelodyGenerator_h



namespace InteractiveTango {
    
    //note -- will need to refactor or recreate another music player class
    //this ugen will send melody notes out via midi -- different than previous
    //BeatTiming *timer, FootOnset *onset
    
    //A faster of implementing this could be just to respond to the OSC messages that the melody player creates.
    //It does create a structure already. -- could add functionally to current interactive tango music player to send the message
    //programmaticlly -- or an inherited music player... the downside is that will have to create a database structure....
    //however, can have some dummy values.
    
    //For now melody generators will be free of orchestral context maybe but it seems like a good idea.
    //Instead -- could respond via OMAX in max/msp -- instead of creating my own melody generator (as I have)
    //hmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
    
    //OK,

    class MelodyGenerator : public UGEN
    {
    private:
        std::vector<MelodyGeneratorAlgorithm *> generators; //a collection of factor oracles <-- refactor into part of a super melodic algorithm class
//        float fo_probability;
        
        std::vector<MappingSchema *> schemas;
        PerceptualEvent *bsevent;
        std::vector<MidiNote> melodyFragment;
        
        bool oneToOneMode; //whether we produce one note per call or else create more notes if busy vs sparse
        
        MelodySection *melodySection; //provides organizatiion
        
        float notesPerUpdate;
        float note_rhythm_ratio_mod;
        float bsMin;
        float bsMax;
        int maxNotesGenerated;
        
        float sparseShortNoteCutOff;

        
    public:
        
        MelodyGenerator( MelodySection *section=NULL,  int _maxNotesGenerated = 5, float _sparseShortNoteCutOff = 1.0f/8.0f)
        {
            oneToOneMode = false; //if yes, ignore profile
//            fo_probability = fp;
            
            setMelodySection(section);
            
            maxNotesGenerated = _maxNotesGenerated;
            sparseShortNoteCutOff = _sparseShortNoteCutOff;
        }
        
        void setMelodySection( MelodySection *section)
        {
            melodySection = section;
            
            if(melodySection!=NULL)
                schemas = melodySection->getMappingSchemas();
            
//            std::cout << "Set melody section. Schemas: " << schemas.size() << std::endl ;
            
            int i =0;
            bool found = false;
            while (!found && i<schemas.size())
            {
                found =  (schemas[i]->getMappingType() == MappingSchema::MappingSchemaType::EVENT) && (
                         (!schemas[i]->getName().compare("Follower Busy Sparse") )  ||  (!schemas[i]->getName().compare("Leader Busy Sparse") ));
                i++;
            }
            if(found)
            {
                bsevent = (PerceptualEvent *)schemas.at(i);
                setMinMaxBusySparse(bsevent->getMinMood(), bsevent->getMaxMood());
                oneToOneMode = false;
            }
            else
            {
                oneToOneMode = true;
            }

        }
        
        void setMinMaxBusySparse(float min, float max)
        {
             bsMin = min;
             bsMax = max;
        }
        
        //TODO -- make agree across files
        double getTicksPerBeat(int i)
        {
            assert(i<generators.size() && i>=0);
            return generators[i]->getTicksPerBeat();
        }
        
        double getBPM(int i=0)
        {
            assert(i<generators.size() && i>=0);
            return generators[i]->getBPM();
        }
        
        void addGeneratorAlgorithm(MelodyGeneratorAlgorithm *alg)
        {
            if(alg->isTrained())
                generators.push_back(alg);
            else
                std::cerr << "MelodyGenerator: Cannot use algorithm! It has not been trained!";
        }
        
        virtual std::vector<ci::osc::Message> getOSC()
        {
            //probably do nothing here... we'll see
            std::vector<ci::osc::Message> msgs;
            return msgs;
        }
        
        virtual bool oneToOne()
        {
            return oneToOneMode;
        }
        
        void turnOn1to1()
        {
            oneToOneMode = true;
        }
        
        
        virtual void update(float bs, float seconds)
        {
            if(oneToOneMode){
                std::cout << "This melody generator is in one to one mode. It does not have perceptual schemas.\n";
                update(seconds);
                return; 
            }
            
           // std::cout << "busy/sparse in generator: " << bs << std::endl;
            
            if(bs == 1)
                notesPerUpdate = 1;
            else
            {
                notesPerUpdate = (int) std::round((((double) std::rand()) / ((double) RAND_MAX) ) * (maxNotesGenerated * (double)bs/(double)bsMax * 0.5 )) +
                std::round(maxNotesGenerated * (double)bs/(double)bsMax * 0.5);
            }
            
            //okay so this maps into a 1-3 thing -- 2, 1, 0.5 -- basically makes notes shorter or longer based on busy sparse
            float which = (float)bs/(float)bsMax ;
            if(which <= 1.0f/3.0f){ note_rhythm_ratio_mod = 2; }
            else {if(which <= 2.0f/3.0f) note_rhythm_ratio_mod = 1; else { note_rhythm_ratio_mod = 0.5; } }
            
            
            for(int i=0; i<notesPerUpdate; i++)
            {
                MidiNote note = generators[0]->generateNext();
                if(i==0) note.tick = 0;
                else note.tick *= note_rhythm_ratio_mod;
                melodyFragment.push_back(note);
            }
            
            //if notes are 1 note per then only play the longer (1/8 or longer...)
            if(notesPerUpdate == 1 && melodyFragment[0].tick > 0 )
            {
                float rhythm_value = melodyFragment[0].tick / generators[0]->getTicksPerBeat();
                while(rhythm_value < sparseShortNoteCutOff)
                {
                    melodyFragment[0] = generators[0]->generateNext();
                }
            }
                
            
//            std::cout <<"bs:" << bs << "  notes per update: " << notesPerUpdate << " rhythm mod: " << note_rhythm_ratio_mod << " which: "<< which <<std::endl;

        }
        
        virtual void update(float seconds=0)
        {
            if(generators.size() <= 0)
            {
                std::cerr << "MelodyGenerator can't generate! No algorithm!\n";
            }
            
            if(oneToOneMode)
                melodyFragment.push_back(generators[0]->generateNext());
            else
            {
                //react to busy/sparse
                std::cout << "This will not react to busy sparse since this generator is being used in one to one mode\n";
            }
            
        }
        
        //also empties out the note buffer.
        virtual std::vector<MidiNote> getCurNotes()
        {
            std::vector<MidiNote> notes(melodyFragment);
            melodyFragment.clear();
            return notes;
        };

    };

}

#endif /* MelodyGenerator_h */
