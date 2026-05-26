#include "pre.h"
#include "ROCAnalysis.h"
#include <FanTomCommons/StudyQualityMetrics.h>

#pragma message костыль, убрать побыстрее
//#include "Q:\projects\AutoStudiesSort\QualityMetrics\StudyLoadSettings.h"

/*!
	\file
	\date 2018/04/20 15:00
	\author kulberg

	\brief 
*/


XRAD_BEGIN





void	ComputeROC(const StudySetTaggingReport	&report, wstring target_folder, wstring testee_id, double cutoff)
{
	auto	doctor_ids = GetTaggingSystemIDs(report);

	if(std::find(doctor_ids.begin(), doctor_ids.end(), testee_id) == doctor_ids.end())
	{
		if(CapsLock())
		{
			auto no = GetButtonDecision(L"Выберите ID проверяемого врача или системы", doctor_ids);
			testee_id = doctor_ids[no];
		}
		else
		{
			if(testee_id.empty() || testee_id == filename_without_extension(StudyLoadSettings::default_testee_id())) ShowString(L"Завершение работы", L"Анализ папок и создание превью (если запрошено) завершено. Тестируемая система не была выбрана. Метрики точности не вычисляются. Нажмите ОК для выхода из системы");
			else Error(L"Выбрана разметка, не соответствующая входным данным. Программа будет завершена.");
			return;
		}
	}

	auto	complete_studies = report.GetStudiesList(L"");
	auto	testee_studies = report.GetStudiesList(testee_id);

// 	if(complete_studies.size() == testee_studies.size())
// 	{
// 		ShowString(L"Испытуемый видел все исследования", ssprintf(L"Всего исследований %zu, отметки внесены в %zu", complete_studies.size(), testee_studies.size()));
// 	}
// 	else
// 	{
// 		ShowString(L"Возможно, испытуемый видел не все исследования", ssprintf(L"Всего исследований %zu, отметки внесены в %zu", complete_studies.size(), testee_studies.size()));
// 	}
#if 0//_DEBUG
	size_t	n_positions = 5;
#else
 	size_t	n_positions = 200;
#endif
	double	min_confidence = 0;
	double	max_confidence = 1.01;
	double	dc = (max_confidence-min_confidence)/(n_positions-1);

	RealFunctionF32	tp(n_positions, 0), tn(n_positions, 0), fp(n_positions, 0), fn(n_positions, 0);
	RealFunctionF32	sen(n_positions, 1), spe(n_positions, 0);

	RealFunctionF32	ppv(n_positions, 1), npv(n_positions, 0);
	RealFunctionF32	acc(n_positions, 0);

	RealFunctionF32	lrp(n_positions, 0);
	RealFunctionF32	lrn(n_positions, 0);

	RealFunctionF32	dices(n_positions, 0);

	GUIProgressBar	pb;

	wstring	cutoff_addition;

	pb.start(L"Идет вычисление метрик", n_positions);
	for(int i = 0; i < n_positions; ++i)
	{
		double	confidence = min_confidence + i*dc;
		double	dice_accumulator(0);
		double	dice_divisor(0);

		pathology_detectioin_metrics	pmetrics = PathologyDetectionMetrics(report, testee_id, confidence);

		for(auto &study_ptr: report)
		{
			dice_metrics	dmetrics = DiceMetrics(study_ptr, testee_id, confidence);

			if(!dmetrics.true_negative())
			{
				dice_accumulator += dmetrics.dice();
				dice_divisor++;
			}
		}

		if(!dice_divisor)
		{
			// полностью негативным исследованиям
			// приписываем дайс 0
			dice_accumulator = 0;
			dice_divisor = 1;
		}

		tp[i] = pmetrics.tp.size();
		tn[i] = pmetrics.tn.size();
		fp[i] = pmetrics.fp.size();
		fn[i] = pmetrics.fn.size();

		double	expert_positive = tp[i]+fn[i];
		double	expert_negative = tn[i]+fp[i];
		double	total_positive = (tp[i] + fp[i]);
		double	total_negative = (tn[i] + fn[i]);

		acc[i] = (tp[i] + tn[i])/(total_positive + total_negative);

		if(i==0)
		{
			// при максимальном значении confidence чувствительность 1, специфичность 0
			sen[i] = 1;
			spe[i] = 0;
			ppv[i] = 1;
			npv[i] = 1;
		}
		else if(i==n_positions-1 || i==n_positions- 2)
		{
			// при максимальном значении confidence чувствительность 0, специфичность 1
			sen[i] = 0;
			spe[i] = 1;
			dices[i] = dices[i-1];

			ppv[i] = ppv[i-1];
			npv[i] = npv[i-1];
		}
		else
		{
			sen[i] = expert_positive ? tp[i]/expert_positive : 0;
			spe[i] = expert_negative ? tn[i]/expert_negative : 0;

			ppv[i] = total_positive ? tp[i]/total_positive : 1;
			npv[i] = total_negative ? tn[i]/total_negative : 0;

			dices[i] = dice_divisor ? dice_accumulator/dice_divisor : 0;
			lrp[i] = sen[i]/(1-spe[i]);
			lrn[i] = (1.-sen[i])/spe[i];
		}


		++pb;
	
//		if(i == cutoff*n_positions && cutoff != 0)
		if(0)
		{
			cutoff_addition += ssprintf(L"\n\nStudies report at confidence cutoff = %.3f:\n", confidence);

			cutoff_addition += ssprintf(L"TP (%zu)\n", pmetrics.tp.size());
			for(auto &id: pmetrics.tp) cutoff_addition += id+L"\n";

			cutoff_addition += ssprintf(L"TN (%zu)\n", pmetrics.tn.size());
			for(auto &id: pmetrics.tn) cutoff_addition += id+L"\n";

			cutoff_addition += ssprintf(L"FP (%zu)\n", pmetrics.fp.size());
			for(auto &id: pmetrics.fp) cutoff_addition += id+L"\n";

			cutoff_addition += ssprintf(L"FN (%zu)\n", pmetrics.fn.size());
			for(auto &id: pmetrics.fn) cutoff_addition += id+L"\n";
		}
	}
	pb.end();

// 	tp[0] = tp[1];
// 	tn[0] = tn[1];
// 	fp[0] = fp[1];
// 	fn[0] = fn[1];

	RealFunctionF32	factors(n_positions, 1);

	for(int i = 0; i < n_positions; ++i)
	{
		double	x = double(i)-n_positions/2;
		double	sigma = n_positions/8;
		factors.at(i) += 0.001*(gauss(x, sigma) - 1);
	}

	auto	spe1 = -spe+1;
	auto	senspe = sen+spe-1;

	double	auc(0);

	auto	find_max = [](const RealFunctionF32 &array)
	{
		size_t	result;
		MaxValue(array, &result);
		if(result >= 1)
		{
			// уходим от точек на границах константных участков
			if(array[result-1] < array[result])
			{
				if(result < array.size()-2)
				{
					if(array[result] <= array[result+1] && array[result] <= array[result+2]) ++result;
				}
				return result;
			}
		}
		return result;
	};

	size_t	max_dice_position = find_max(dices*factors);
	size_t	max_senspe_position = find_max(senspe*factors);
	size_t	preferred_confidence_position = 0;

	if(dices[max_senspe_position] >= dices[max_dice_position] && senspe[max_senspe_position] >= senspe[max_dice_position])
	{
		preferred_confidence_position = max_senspe_position;
	}
	else if(dices[max_senspe_position] <= dices[max_dice_position] && senspe[max_senspe_position] <= senspe[max_dice_position])
	{
		preferred_confidence_position = max_dice_position;
	}
	else
	{
		preferred_confidence_position = max_senspe_position;
	}


	string	message;

	message += ssprintf("Testee system id\t%s", convert_to_string(testee_id).c_str());

	for(size_t i = 1; i < n_positions; ++i)
	{
		if(spe1[i-1] != spe1[i])
		{
			auc += 0.5*(sen[i] + sen[i-1])*(spe1[i-1] - spe1[i]);
		}
	}

	message += ssprintf("\nAUC\t%.3f", auc);

	double	max_senspe_confidence = min_confidence + max_senspe_position*dc;
	double max_dice_confidence = min_confidence + max_dice_position*dc;
	double	preferred_confidence = min_confidence + preferred_confidence_position*dc;

	auto	print_metrics_at_position = [&sen, &spe, &fp, &tp, &fn, &tn, &ppv, &npv, &dices, &acc, &lrp, &lrn](size_t position, double confidence_value, string header_legend)
	{
		string result;

		result += "\n";
		result += header_legend + ssprintf("\t%.3f", confidence_value);
		result += "\nMetrics for this position:";
		result += ssprintf("\nsen\t%.2f", sen[position]);
		result += ssprintf("\nspe\t%.2f", spe[position]);
		result += ssprintf("\nppv\t%.2f", ppv[position]);
		result += ssprintf("\nnpv\t%.2f", npv[position]);

		result += ssprintf("\nacc\t%.2f", acc[position]);
		result += ssprintf("\nlrp\t%.2f", lrp[position]);
		result += ssprintf("\nlrn\t%.2f", lrn[position]);

		result += ssprintf("\nfp\t%g", fp[position]);
		result += ssprintf("\nfn\t%g", fn[position]);
		result += ssprintf("\ntp\t%g", tp[position]);
		result += ssprintf("\ntn\t%g", tn[position]);
		result += ssprintf("\nDice\t%.2f", dices[position]);

		return result;
	};



	{

		message += print_metrics_at_position(0, 0, "\nConfidence value = ");
		message += print_metrics_at_position(max_senspe_position, max_senspe_confidence, "\nMaximum sensitivity/specificity reached at study confidence threshold");
		message += print_metrics_at_position(max_dice_position, max_dice_confidence, "\nMaximum Dice reached at ROI onfidence threshold");
		message += print_metrics_at_position(preferred_confidence_position, preferred_confidence, "\nRecommended confidence threshold");
	}



	std::replace(message.begin(), message.end(), '.', ',');
	
	printf("\n\n");
	printf(message.c_str());

	fflush(stdout);

	static GraphSet	dice_gs(testee_id + L"Dices", L"Dice", L"Confidence threshold");
	static GraphSet	ROC(testee_id + L"ROC", L"True positive rate", L"False positive rate");
	static GraphSet	senspe_gs(testee_id + L"Sensitivity and specificity dynamics", L"sensitivity and specificity", L"Confidence threshold");
	static GraphSet	negpos_gs(testee_id + L"Negative and positive counters", L"cases", L"Confidence threshold");
	static GraphSet	ppv_gs(testee_id + L"PPV and NPV dynamics", L"PPV and NPV", L"Confidence threshold");

	auto	thickness(2.5);
	auto	epsilon = 0.025;
	auto	style = dashed_color_lines;

	auto	graph_range = range2_F32(-epsilon, -epsilon, 1+epsilon, 1+epsilon);


	ROC.SetWindowTitle(L"ROC " + testee_id);
	dice_gs.SetWindowTitle(L"Dices " + testee_id);
	senspe_gs.SetWindowTitle(L"senspe " + testee_id);

	senspe_gs.ChangeGraphUniform(0, sen, min_confidence, dc, "sen");
	senspe_gs.ChangeGraphUniform(1, spe, min_confidence, dc, "spe");
	senspe_gs.ChangeGraphUniform(2, senspe, min_confidence, dc, "sen+spe-1");

	negpos_gs.ChangeGraphUniform(0, tp, min_confidence, dc, "tp");
	negpos_gs.ChangeGraphUniform(1, tn, min_confidence, dc, "tn");
	negpos_gs.ChangeGraphUniform(2, fp, min_confidence, dc, "fp");
	negpos_gs.ChangeGraphUniform(3, fn, min_confidence, dc, "fn");
	negpos_gs.ChangeGraphUniform(4, tp+tn+fp+fn, min_confidence, dc, "tp+tn+fp+fn");

	ppv_gs.ChangeGraphUniform(0, ppv, min_confidence, dc, "ppv");
	ppv_gs.ChangeGraphUniform(1, npv, min_confidence, dc, "npv");

	dice_gs.ChangeGraphUniform(0, dices, min_confidence, dc, "Dice");
	ROC.ChangeGraphParametric(0, sen, spe1, "ROC");
	
	dice_gs.SetGraphStyle(style, thickness);
	ROC.SetGraphStyle(style, thickness);
	senspe_gs.SetGraphStyle(style, thickness);

	ppv_gs.SetScale(graph_range);
	dice_gs.SetScale(graph_range);
	ROC.SetScale(graph_range);
	senspe_gs.SetScale(graph_range);


	dice_gs.Display(false);
	senspe_gs.Display(false);
	ROC.Display(false);
	negpos_gs.Display(false);
	ppv_gs.Display(false);

	if(!DirectoryExists(target_folder)) CreatePath(target_folder);
	wstring filename = target_folder + L"/" + testee_id;

	text_file_writer	report_csv(filename + L"_metrics.csv", text_encoding::utf16_le);

	auto wmessage = convert_to_wstring(message);
	wmessage += cutoff_addition;

	report_csv.printf_(wmessage);
	report_csv.flush();
	ROC.SavePicture(filename+L"_ROC.png");

	ShowFloating("AUC", auc);
}




XRAD_END

