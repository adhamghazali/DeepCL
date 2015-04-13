// Copyright Hugh Perkins 2015 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License, 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

#include "NormalizationHelper.h"
#include "NeuralNet.h"
#include "AccuracyHelper.h"
#include "Trainable.h"

#include "BatchLearner.h"

using namespace std;

#undef STATIC
#undef VIRTUAL
#define STATIC
#define VIRTUAL

template< typename T>
void NetLearnLabeledBatch<T>::run( Trainable *net, T *batchData, int const*batchLabels ) {
//    cout << "NetLearnLabeledBatch learningrate=" << learningRate << endl;
    net->learnBatchFromLabels( learningRate, batchData, batchLabels );
}

template< typename T>
void NetPropagateBatch<T>::run( Trainable *net, T *batchData, int const*batchLabels ) {
//    cout << "NetPropagateBatch" << endl;
    net->propagate( batchData );
}

template< typename T>
void NetBackpropBatch<T>::run( Trainable *net, T *batchData, int const*batchLabels ) {
//    cout << "NetBackpropBatch learningrate=" << learningRate << endl;
    net->backPropFromLabels( learningRate, batchLabels );
}

template< typename T > BatchLearner<T>::BatchLearner( Trainable *net ) :
    net( net ) {
}

template< typename T > VIRTUAL void BatchLearner<T>::addPostBatchAction( PostBatchAction *action ) {
    postBatchActions.push_back( action );
}

template< typename T > EpochResult BatchLearner<T>::batchedNetAction( int batchSize, int N, T *data, int const*labels, NetAction<T> *netAction ) {
    int numRight = 0;
    float loss = 0;
    int thisBatchSize = batchSize;
    net->setBatchSize( batchSize );
    int numBatches = (N + batchSize - 1 ) / batchSize;
    int inputCubeSize = net->getInputCubeSize();
    for( int batch = 0; batch < numBatches; batch++ ) {
        int batchStart = batch * batchSize;
        if( batch == numBatches - 1 ) {
            thisBatchSize = N - batchStart;
            net->setBatchSize( thisBatchSize );
        }
        netAction->run( net, &(data[ batchStart * inputCubeSize ]), &(labels[batchStart]) );
        loss += net->calcLossFromLabels( &(labels[batchStart]) );
        int thisNumRight = net->calcNumRight( &(labels[batchStart]) );
        numRight += thisNumRight;
//        cout << "batchlearner batch=" << batch << " thisbatchsize=" << thisBatchSize << " thisnumright " << thisNumRight << " numright=" << numRight << " batchstart=" << batchStart << endl;
        for( vector<PostBatchAction *>::iterator it = postBatchActions.begin(); it != postBatchActions.end(); it++ ) {
            (*it)->run( batch, loss, numRight );
        }
    }
    EpochResult epochResult( loss, numRight );
    return epochResult;
}

template< typename T > int BatchLearner<T>::test( int batchSize, int N, T *testData, int const*testLabels ) {
    net->setTraining( false );
    NetAction<T> *action = new NetPropagateBatch<T>();
    int numRight = batchedNetAction( batchSize, N, testData, testLabels, action ).numRight;
    delete action;
    return numRight;
}

template< typename T > int BatchLearner<T>::propagateForTrain( int batchSize, int N, T *data, int const*labels ) {
    net->setTraining( true );
    NetAction<T> *action = new NetPropagateBatch<T>();
    int numRight = batchedNetAction( batchSize, N, data, labels, action ).numRight;
    delete action;
    return numRight;
}

template< typename T > EpochResult BatchLearner<T>::backprop( float learningRate, int batchSize, int N, T *data, int const*labels ) {
    net->setTraining( true );
    NetAction<T> *action = new NetBackpropBatch<T>( learningRate );
    EpochResult epochResult = batchedNetAction( batchSize, N, data, labels, action );
    delete action;
    return epochResult;
}

template< typename T > EpochResult BatchLearner<T>::runEpochFromLabels( float learningRate, int batchSize, int Ntrain, T *trainData, int const*trainLabels ) {
    net->setTraining( true );
    NetAction<T> *action = new NetLearnLabeledBatch<T>( learningRate );
    EpochResult epochResult = batchedNetAction( batchSize, Ntrain, trainData, trainLabels, action );
    delete action;
    return epochResult;
}

template< typename T > float BatchLearner<T>::runEpochFromExpected( float learningRate, int batchSize, int N, T *data, float *expectedResults ) {
    net->setTraining( true );
    float loss = 0;
    net->setBatchSize( batchSize );
    const int numBatches = (N + batchSize - 1 ) / batchSize;
    const int inputCubeSize = net->getInputCubeSize();
    const int outputCubeSize = net->getOutputCubeSize();
    for( int batch = 0; batch < numBatches; batch++ ) {
        int batchStart = batch * batchSize;
        if( batch == numBatches - 1 ) {
            net->setBatchSize( N - batchStart );
        }
        net->learnBatch( learningRate, &(data[ batchStart * inputCubeSize ]), &(expectedResults[batchStart * outputCubeSize]) );
        loss += net->calcLoss( &( expectedResults[batchStart * outputCubeSize]) );
    }
    return loss;
}

template class BatchLearner<unsigned char>;
template class BatchLearner<float>;

