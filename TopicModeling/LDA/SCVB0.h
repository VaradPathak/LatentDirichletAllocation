/*
 * SCVB0.h
 *
 *  Created on: Mar 7, 2014
 *      Author: Varad Pathak, Arul Samuel
 */

#ifndef SCVB0_H_
#define SCVB0_H_

#include <vector>
#include "Document.h"
#include <iostream>

using namespace std;

class SCVB0 {
public:
	int iterations;
	int K; //Number of Topics
	int W; //no of terms in vocab
	int D; //Total no of docs in corpus
	int d; //Number of documents to be processed by each mini-batch
	int C; //Total no of words in corpus
	intMap Cj; // Number of words in jth Doc
	int M; //number of documents in Mini-Batch

	float alpha;
	float eta;

	int s;
	int tau;
	float kappa;
	int t;

	float rhoPhi;
	float rhoTheta;

	SCVB0(int iter, int numberOfTopics, int vocabSize, int numOfDocs,
			int corpusSize);
	void run(vector<Document> docVector);
};

#endif /* SCVB0_H_ */
