/*****************
 Copyright 2011, 2012 Mark Higgins
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 *****************/

#include <cmath>
#include <fstream>
#include <string>
#include "strategypubeval.h"
#include "gamefns.h"



strategyPubEval::strategyPubEval( bool randomWeights, bool valueIsEquity, double alpha ) : valueIsEquity(valueIsEquity), alpha(alpha)
{
    // by default, no learning
    
    learning = false;
    if( randomWeights )
        initializeRandomWeights();
    else
        initializeTesauroWeights();
}

void strategyPubEval::initializeRandomWeights()
{
    weightsContact.resize(122);
    weightsRace.resize(122);
    CRandomMersenne rng(1);
    
    for( int i=0; i<122; i++ )
    {
        weightsContact.at(i) = rng.IRandom(-100, 100)/1000.;
        weightsRace.at(i) = rng.IRandom(-100, 100)/1000.;
    }
}

void strategyPubEval::initializeTesauroWeights()
{
    // initialize the weights for boards in contact (not a race)
    
    weightsContact.resize(122);
    weightsContact[0] = 0.25696;
    weightsContact[1] = -0.66937;
    weightsContact[2] = -1.66135;
    weightsContact[3] = -2.02487;
    weightsContact[4] = -2.53398;
    weightsContact[5] = -0.16092;
    weightsContact[6] = -1.11725;
    weightsContact[7] = -1.06654;
    weightsContact[8] = -0.9283;
    weightsContact[9] = -1.99558;
    weightsContact[10] = -1.10388;
    weightsContact[11] = -0.80802;
    weightsContact[12] = 0.09856;
    weightsContact[13] = -0.62086;
    weightsContact[14] = -1.27999;
    weightsContact[15] = -0.5922;
    weightsContact[16] = -0.73667;
    weightsContact[17] = 0.89032;
    weightsContact[18] = -0.38933;
    weightsContact[19] = -1.59847;
    weightsContact[20] = -1.50197;
    weightsContact[21] = -0.60966;
    weightsContact[22] = 1.56166;
    weightsContact[23] = -0.47389;
    weightsContact[24] = -1.8039;
    weightsContact[25] = -0.83425;
    weightsContact[26] = -0.97741;
    weightsContact[27] = -1.41371;
    weightsContact[28] = 0.245;
    weightsContact[29] = 0.1097;
    weightsContact[30] = -1.36476;
    weightsContact[31] = -1.05572;
    weightsContact[32] = 1.1542;
    weightsContact[33] = 0.11069;
    weightsContact[34] = -0.38319;
    weightsContact[35] = -0.74816;
    weightsContact[36] = -0.59244;
    weightsContact[37] = 0.81116;
    weightsContact[38] = -0.39511;
    weightsContact[39] = 0.11424;
    weightsContact[40] = -0.73169;
    weightsContact[41] = -0.56074;
    weightsContact[42] = 1.09792;
    weightsContact[43] = 0.15977;
    weightsContact[44] = 0.13786;
    weightsContact[45] = -1.18435;
    weightsContact[46] = -0.43363;
    weightsContact[47] = 1.06169;
    weightsContact[48] = -0.21329;
    weightsContact[49] = 0.04798;
    weightsContact[50] = -0.94373;
    weightsContact[51] = -0.22982;
    weightsContact[52] = 1.22737;
    weightsContact[53] = -0.13099;
    weightsContact[54] = -0.06295;
    weightsContact[55] = -0.75882;
    weightsContact[56] = -0.13658;
    weightsContact[57] = 1.78389;
    weightsContact[58] = 0.30416;
    weightsContact[59] = 0.36797;
    weightsContact[60] = -0.69851;
    weightsContact[61] = 0.13003;
    weightsContact[62] = 1.2307;
    weightsContact[63] = 0.40868;
    weightsContact[64] = -0.21081;
    weightsContact[65] = -0.64073;
    weightsContact[66] = 0.31061;
    weightsContact[67] = 1.59554;
    weightsContact[68] = 0.65718;
    weightsContact[69] = 0.25429;
    weightsContact[70] = -0.80789;
    weightsContact[71] = 0.0824;
    weightsContact[72] = 1.78964;
    weightsContact[73] = 0.54304;
    weightsContact[74] = 0.41174;
    weightsContact[75] = -1.06161;
    weightsContact[76] = 0.07851;
    weightsContact[77] = 2.01451;
    weightsContact[78] = 0.49786;
    weightsContact[79] = 0.91936;
    weightsContact[80] = -0.9075;
    weightsContact[81] = 0.05941;
    weightsContact[82] = 1.8312;
    weightsContact[83] = 0.58722;
    weightsContact[84] = 1.28777;
    weightsContact[85] = -0.83711;
    weightsContact[86] = -0.33248;
    weightsContact[87] = 2.64983;
    weightsContact[88] = 0.52698;
    weightsContact[89] = 0.82132;
    weightsContact[90] = -0.58897;
    weightsContact[91] = -1.18223;
    weightsContact[92] = 3.35809;
    weightsContact[93] = 0.62017;
    weightsContact[94] = 0.57353;
    weightsContact[95] = -0.07276;
    weightsContact[96] = -0.36214;
    weightsContact[97] = 4.37655;
    weightsContact[98] = 0.45481;
    weightsContact[99] = 0.21746;
    weightsContact[100] = 0.10504;
    weightsContact[101] = -0.61977;
    weightsContact[102] = 3.54001;
    weightsContact[103] = 0.04612;
    weightsContact[104] = -0.18108;
    weightsContact[105] = 0.63211;
    weightsContact[106] = -0.87046;
    weightsContact[107] = 2.47673;
    weightsContact[108] = -0.48016;
    weightsContact[109] = -1.27157;
    weightsContact[110] = 0.86505;
    weightsContact[111] = -1.11342;
    weightsContact[112] = 1.24612;
    weightsContact[113] = -0.82385;
    weightsContact[114] = -2.77082;
    weightsContact[115] = 1.23606;
    weightsContact[116] = -1.59529;
    weightsContact[117] = 0.10438;
    weightsContact[118] = -1.30206;
    weightsContact[119] = -4.1152;
    weightsContact[120] = 5.62596;
    weightsContact[121] = -2.758;
    
    // then the race weights
    
    weightsRace.resize(122);
    weightsRace[0] = 0.0;
    weightsRace[1] = -0.1716;
    weightsRace[2] = 0.2701;
    weightsRace[3] = 0.29906;
    weightsRace[4] = -0.08471;
    weightsRace[5] = 0.0;
    weightsRace[6] = -1.40375;
    weightsRace[7] = -1.05121;
    weightsRace[8] = 0.07217;
    weightsRace[9] = -0.01351;
    weightsRace[10] = 0.0;
    weightsRace[11] = -1.29506;
    weightsRace[12] = -2.16183;
    weightsRace[13] = 0.13246;
    weightsRace[14] = -1.03508;
    weightsRace[15] = 0.0;
    weightsRace[16] = -2.29847;
    weightsRace[17] = -2.34631;
    weightsRace[18] = 0.17253;
    weightsRace[19] = 0.08302;
    weightsRace[20] = 0.0;
    weightsRace[21] = -1.27266;
    weightsRace[22] = -2.87401;
    weightsRace[23] = -0.07456;
    weightsRace[24] = -0.3424;
    weightsRace[25] = 0.0;
    weightsRace[26] = -1.3464;
    weightsRace[27] = -2.46556;
    weightsRace[28] = -0.13022;
    weightsRace[29] = -0.01591;
    weightsRace[30] = 0.0;
    weightsRace[31] = 0.27448;
    weightsRace[32] = 0.60015;
    weightsRace[33] = 0.48302;
    weightsRace[34] = 0.25236;
    weightsRace[35] = 0.0;
    weightsRace[36] = 0.39521;
    weightsRace[37] = 0.68178;
    weightsRace[38] = 0.05281;
    weightsRace[39] = 0.09266;
    weightsRace[40] = 0.0;
    weightsRace[41] = 0.24855;
    weightsRace[42] = -0.06844;
    weightsRace[43] = -0.37646;
    weightsRace[44] = 0.05685;
    weightsRace[45] = 0.0;
    weightsRace[46] = 0.17405;
    weightsRace[47] = 0.0043;
    weightsRace[48] = 0.74427;
    weightsRace[49] = 0.00576;
    weightsRace[50] = 0.0;
    weightsRace[51] = 0.12392;
    weightsRace[52] = 0.31202;
    weightsRace[53] = -0.91035;
    weightsRace[54] = -0.1627;
    weightsRace[55] = 0.0;
    weightsRace[56] = 0.01418;
    weightsRace[57] = -0.10839;
    weightsRace[58] = -0.02781;
    weightsRace[59] = -0.88035;
    weightsRace[60] = 0.0;
    weightsRace[61] = 1.07274;
    weightsRace[62] = 2.00366;
    weightsRace[63] = 1.16242;
    weightsRace[64] = 0.2252;
    weightsRace[65] = 0.0;
    weightsRace[66] = 0.85631;
    weightsRace[67] = 1.06349;
    weightsRace[68] = 1.49549;
    weightsRace[69] = 0.18966;
    weightsRace[70] = 0.0;
    weightsRace[71] = 0.37183;
    weightsRace[72] = -0.50352;
    weightsRace[73] = -0.14818;
    weightsRace[74] = 0.12039;
    weightsRace[75] = 0.0;
    weightsRace[76] = 0.13681;
    weightsRace[77] = 0.13978;
    weightsRace[78] = 1.11245;
    weightsRace[79] = -0.12707;
    weightsRace[80] = 0.0;
    weightsRace[81] = -0.22082;
    weightsRace[82] = 0.20178;
    weightsRace[83] = -0.06285;
    weightsRace[84] = -0.52728;
    weightsRace[85] = 0.0;
    weightsRace[86] = -0.13597;
    weightsRace[87] = -0.19412;
    weightsRace[88] = -0.09308;
    weightsRace[89] = -1.26062;
    weightsRace[90] = 0.0;
    weightsRace[91] = 3.05454;
    weightsRace[92] = 5.16874;
    weightsRace[93] = 1.5068;
    weightsRace[94] = 5.35;
    weightsRace[95] = 0.0;
    weightsRace[96] = 2.19605;
    weightsRace[97] = 3.8539;
    weightsRace[98] = 0.88296;
    weightsRace[99] = 2.30052;
    weightsRace[100] = 0.0;
    weightsRace[101] = 0.92321;
    weightsRace[102] = 1.08744;
    weightsRace[103] = -0.11696;
    weightsRace[104] = -0.7856;
    weightsRace[105] = 0.0;
    weightsRace[106] = -0.09795;
    weightsRace[107] = -0.8305;
    weightsRace[108] = -1.09167;
    weightsRace[109] = -4.94251;
    weightsRace[110] = 0.0;
    weightsRace[111] = -1.00316;
    weightsRace[112] = -3.66465;
    weightsRace[113] = -2.56906;
    weightsRace[114] = -9.67677;
    weightsRace[115] = 0.0;
    weightsRace[116] = -2.77982;
    weightsRace[117] = -7.26713;
    weightsRace[118] = -3.40177;
    weightsRace[119] = -12.32252;
    weightsRace[120] = 0.0;
    weightsRace[121] = 3.4204;
}

strategyPubEval::~strategyPubEval()
{
}

vector<double> pubEvalInputs( const board& brd )
{
    vector<double> inputs(122,0.);

    int j, n;
    
    // first encode the board position in reverse order, from 24->1. Player
    // and opponent counts are grouped together, with player using positive
    // counts and opponent using negative.
    
    for( j=0; j<24; j++ ) 
    {
        n = brd.checker(23-j) - brd.otherChecker(23-j);
        if (n != 0) 
        {
            if (n == -1)
                inputs.at( 5 * j + 0 ) = 1.0;
            if (n == 1)
                inputs.at( 5 * j + 1 ) = 1.0;
            if (n >= 2)
                inputs.at( 5 * j + 2 ) = 1.0;
            if (n == 3)
                inputs.at( 5 * j + 3 ) = 1.0;
            if (n >= 4)
                inputs.at( 5 * j + 4 ) = (float) (n - 3) / 2.0;
        }
    }
    
    // then the number of opponent hit and the number of player's borne in
    
    inputs.at( 120 ) = brd.otherHit() / 2.;
    inputs.at( 121 ) = brd.bornIn() / 15.;
    
    return inputs;
}

double strategyPubEval::boardValue( const board& brd, const hash_map<string,int>* context )
{
    if( valueIsEquity )
    {
        if( brd.bornIn() == 15 )
        {
            if( brd.otherBornIn() == 0 )
            {
                if( brd.otherNoBackgammon() )
                    return 2;
                else
                    return 3;
            }
            else
                return 1;
        }
        else if( brd.otherBornIn() == 15 )
        {
            if( brd.bornIn() == 0 )
            {
                if( brd.noBackgammon() )
                    return -2;
                else
                    return -3;
            }
            else
                return -1;
        }
    }
    else
    {
        // if all the pieces are in, return a high score
        
        if( brd.bornIn() == 15 ) return 1e22;
    }
    
    // figure out which weights we're using. This comes from the context, not the current board,
    // because we need to make sure that when we're comparing possible moves based on the original
    // board, all possible moves are evaluated using the same weights (since the two sets of weights
    // don't give similar values).
    
    const vector<double> * weights;
    bool isRace;
    if( context == 0 ) throw "Need to have context to decide whether the network is race or contact";
    hash_map<string,int>::const_iterator it=context->find( "isRace" );
    if( it == context->end() ) throw "Cannot find isRace key in context hash";
    isRace = ( it->second == 1 );
    
    if( isRace )
        weights = &weightsRace;
    else
        weights = &weightsContact;
    
    // get the inputs from the board
    
    vector<double> inputs = pubEvalInputs( brd );
    //if( inputs.size() != weights->size() ) throw string( "Number of inputs does not match number of weights" );
    
    // calculate sum( weights * inputs ) - this is the board value
    
    double sum=0;
    for( int i=0; i<inputs.size(); i++ )
        sum += weights->at(i) * inputs.at(i);
    return sum;
}

hash_map<string,int> strategyPubEval::boardContext( const board& brd ) const
{
    // add a bit of context that defines whether the board is in race or contact
    
    hash_map<string,int> map;
    map[ "isRace" ] = brd.isRace() ? 1 : 0;
    return map;
}

bool strategyPubEval::needsUpdate() const
{
    return learning;
}

void strategyPubEval::update( const board& oldBoard, const board& newBoard )
{
    if( !valueIsEquity ) throw string( "Training only works if board value is equity" );
    
    // calculate the equity from the new board state, using the state from the old board
    
    hash_map<string,int> ctx;
    ctx["isRace"] = oldBoard.isRace() ? 1 : 0;
    
    double newEquity=boardValue(newBoard,&ctx);
    
    // use this as a proxy for supervised learning
    
    updateFromEquity( oldBoard, newEquity );
}

void strategyPubEval::updateFromEquity( const board& brd, double equity )
{
    vector<double> * weights;
    if( brd.isRace() )
        weights = &weightsRace;
    else
        weights = &weightsContact;
    
    // get the estimated equity
    
    hash_map<string,int> ctx;
    ctx["isRace"] = brd.isRace() ? 1 : 0;
    double estEquity=boardValue(brd,&ctx);
    
    // update weights
    
    vector<double> inputs = pubEvalInputs(brd);
    
    for( int i=0; i<122; i++ )
        weights->at(i) += alpha * ( equity - estEquity ) * inputs.at(i);
}

void strategyPubEval::writeWeights( const string& filePrefix, bool writeContact, bool writeRace )
{
    string contactName = filePrefix + "_contact.txt";
    string raceName = filePrefix + "_race.txt";
    if( writeContact )
    {
        ofstream f( contactName.c_str() );
        for( int i=0; i<122; i++ )
            f << weightsContact.at(i) << endl;
        f.close();
    }
    if( writeRace )
    {
        ofstream f( raceName.c_str() );
        for( int i=0; i<122; i++ )
            f << weightsRace.at(i) << endl;
        f.close();
    }
}

void strategyPubEval::loadWeights( const string& filePrefix )
{
    string contactName = filePrefix + "_contact.txt";
    string raceName = filePrefix + "_race.txt";
    ifstream f( contactName.c_str(), ios::in );
    if( !f ) throw string( "No file named " + contactName );    
    weightsContact.resize(122);
    string line;
    for( int i=0; i<122; i++ )
    {
        getline( f, line );
        weightsContact.at(i) = atof( line.c_str() );
    }
    f.close();
    f.open( raceName.c_str(), ios::in );
    if( !f ) throw string( "No file named " + raceName );    
    weightsRace.resize(122);
    for( int i=0; i<122; i++ )
    {
        getline( f, line );
        weightsRace.at(i) = atof( line.c_str() );
    }
}
