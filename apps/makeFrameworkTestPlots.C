void makeFrameworkTestPlots(TString filename, TString label, double ymin, double ymax) {
	TFile *f = new TFile(filename, "READ");

	TGraphErrors *root_graph;
	TGraphErrors *mygraph;

	f->GetObject("root_graph", root_graph);
	f->GetObject("graph", mygraph);

	TCanvas *c = new TCanvas("c", "", 1000, 700);
	if (root_graph && mygraph) {
		root_graph->SetTitle(label);
		root_graph->Draw("APC");
		root_graph->GetYaxis()->SetRangeUser(ymin, ymax);
		root_graph->GetYaxis()->SetTitle(
				"mean((gaussfit_{amp}-Amp_{ref})/Amp_{ref}) [%]");
		root_graph->GetYaxis()->SetTitleOffset(1.4);
		root_graph->GetXaxis()->SetTitle("lower fit range");
		root_graph->GetXaxis()->SetTitleOffset(1.4);
		mygraph->Draw("PCSAME");
		mygraph->SetLineColor(2);
		mygraph->SetMarkerColor(2);
		gPad->Update();
		TLine *line = new TLine(gPad->GetUxmin(), 0.0, gPad->GetUxmax(), 0.0);
		line->Draw();
		root_graph->Draw("PCSAME");

		gPad->SetTopMargin(0.05);
		gPad->SetRightMargin(0.05);

		/*gPad->GetUxmin() + 0.8 * (gPad->GetUxmax() - gPad->GetUxmin()),
		gPad->GetUymin() + 0.8 * (gPad->GetUymax() - gPad->GetUymin()),
		gPad->GetUxmin() + 1.0 * (gPad->GetUxmax() - gPad->GetUxmin()),
		gPad->GetUymin() + 1.0 * (gPad->GetUymax() - gPad->GetUymin()));*/

		TLegend *leg = new TLegend(0.8, 0.8, 0.95, 0.95);
		leg->AddEntry(root_graph, "root fit result", "lp");
		leg->AddEntry(mygraph, "my fit result", "lp");
		leg->Draw();
		c->SaveAs(label + "_frameworktest.pdf");
	}
}
